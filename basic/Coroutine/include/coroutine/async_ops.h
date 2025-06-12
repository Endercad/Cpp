#pragma once

#include <coroutine>
#include <chrono>
#include <thread>
#include <future>
#include <fstream>
#include <string>
#include <functional>

/**
 * @brief 异步操作工具集
 * 
 * 这个文件包含了各种常用的异步操作的awaitable包装，
 * 展示了如何将传统的异步API适配到协程中。
 */

/**
 * @brief 延迟执行awaitable
 * 
 * 这个类展示了如何创建一个基于时间的awaitable，
 * 类似于JavaScript的setTimeout或C#的Task.Delay
 */
class Delay {
public:
    /**
     * @brief 构造函数
     * @param duration 延迟时间
     */
    explicit Delay(std::chrono::milliseconds duration) 
        : duration_(duration) {}
    
    /**
     * @brief await_ready() - 检查是否立即可用
     * 
     * 对于延迟操作，总是返回false，表示需要等待
     */
    bool await_ready() const noexcept { 
        return false; 
    }
    
    /**
     * @brief await_suspend() - 挂起协程并启动异步操作
     * 
     * 这里启动一个异步定时器，定时器到期后恢复协程执行
     * 
     * @param handle 当前被挂起的协程句柄
     */
    void await_suspend(std::coroutine_handle<> handle) {
        // 在新线程中执行延迟操作
        std::thread([handle, duration = duration_]() {
            std::this_thread::sleep_for(duration);
            handle.resume();  // 延迟结束后恢复协程
        }).detach();
    }
    
    /**
     * @brief await_resume() - 获取操作结果
     * 
     * 对于延迟操作，没有返回值
     */
    void await_resume() const noexcept {}

private:
    std::chrono::milliseconds duration_;
};

/**
 * @brief 异步文件读取awaitable
 * 
 * 这个类展示了如何将文件I/O操作包装成awaitable
 */
class AsyncFileRead {
public:
    /**
     * @brief 构造函数
     * @param filename 要读取的文件名
     */
    explicit AsyncFileRead(const std::string& filename) 
        : filename_(filename) {}
    
    bool await_ready() const noexcept { 
        return false; 
    }
    
    /**
     * @brief await_suspend() - 启动异步文件读取
     * 
     * 使用std::async在后台线程中执行文件读取操作
     */
    void await_suspend(std::coroutine_handle<> handle) {
        // 使用std::async启动异步文件读取
        future_ = std::async(std::launch::async, [this, handle]() {
            try {
                std::ifstream file(filename_);
                if (!file.is_open()) {
                    throw std::runtime_error("无法打开文件: " + filename_);
                }
                
                // 读取整个文件内容
                std::string content;
                std::string line;
                while (std::getline(file, line)) {
                    content += line + '\n';
                }
                
                result_ = std::move(content);
                success_ = true;
            } catch (...) {
                exception_ = std::current_exception();
                success_ = false;
            }
            
            // 文件读取完成后恢复协程
            handle.resume();
        });
    }
    
    /**
     * @brief await_resume() - 获取文件内容
     * 
     * 如果读取过程中发生异常，则重新抛出异常
     */
    std::string await_resume() {
        if (!success_) {
            std::rethrow_exception(exception_);
        }
        return std::move(result_);
    }

private:
    std::string filename_;
    std::string result_;
    bool success_ = false;
    std::exception_ptr exception_;
    std::future<void> future_;
};

/**
 * @brief 异步文件写入awaitable
 */
class AsyncFileWrite {
public:
    /**
     * @brief 构造函数
     * @param filename 要写入的文件名
     * @param content 要写入的内容
     */
    AsyncFileWrite(const std::string& filename, const std::string& content)
        : filename_(filename), content_(content) {}
    
    bool await_ready() const noexcept { 
        return false; 
    }
    
    void await_suspend(std::coroutine_handle<> handle) {
        future_ = std::async(std::launch::async, [this, handle]() {
            try {
                std::ofstream file(filename_);
                if (!file.is_open()) {
                    throw std::runtime_error("无法创建文件: " + filename_);
                }
                
                file << content_;
                file.flush();
                
                success_ = true;
            } catch (...) {
                exception_ = std::current_exception();
                success_ = false;
            }
            
            handle.resume();
        });
    }
    
    void await_resume() {
        if (!success_) {
            std::rethrow_exception(exception_);
        }
    }

private:
    std::string filename_;
    std::string content_;
    bool success_ = false;
    std::exception_ptr exception_;
    std::future<void> future_;
};

/**
 * @brief 通用异步操作包装器
 * 
 * 这个模板类可以将任何返回std::future的操作包装成awaitable
 */
template<typename T>
class AsyncOperation {
public:
    /**
     * @brief 构造函数
     * @param func 返回std::future<T>的函数
     */
    template<typename Func>
    explicit AsyncOperation(Func&& func) 
        : future_(std::forward<Func>(func)()) {}
    
    /**
     * @brief await_ready() - 检查future是否已准备好
     */
    bool await_ready() const noexcept {
        return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }
    
    /**
     * @brief await_suspend() - 在后台等待future完成
     */
    void await_suspend(std::coroutine_handle<> handle) {
        std::thread([this, handle]() {
            future_.wait();  // 等待future完成
            handle.resume();  // 恢复协程
        }).detach();
    }
    
    /**
     * @brief await_resume() - 获取future的结果
     */
    T await_resume() {
        return future_.get();
    }

private:
    std::future<T> future_;
};

/**
 * @brief 创建延迟awaitable的辅助函数
 * 
 * 使用示例：co_await delay(std::chrono::milliseconds(1000));
 */
inline Delay delay(std::chrono::milliseconds duration) {
    return Delay{duration};
}

/**
 * @brief 创建异步文件读取awaitable的辅助函数
 * 
 * 使用示例：std::string content = co_await read_file("test.txt");
 */
inline AsyncFileRead read_file(const std::string& filename) {
    return AsyncFileRead{filename};
}

/**
 * @brief 创建异步文件写入awaitable的辅助函数
 * 
 * 使用示例：co_await write_file("output.txt", "Hello World");
 */
inline AsyncFileWrite write_file(const std::string& filename, const std::string& content) {
    return AsyncFileWrite{filename, content};
}

/**
 * @brief 创建通用异步操作awaitable的辅助函数
 * 
 * 使用示例：
 * auto result = co_await async_op([]() {
 *     return std::async(std::launch::async, []() { return 42; });
 * });
 */
template<typename Func>
auto async_op(Func&& func) {
    using ReturnType = typename std::invoke_result_t<Func>::value_type;
    return AsyncOperation<ReturnType>{std::forward<Func>(func)};
} 