#pragma once

#include <coroutine>
#include <exception>
#include <variant>
#include <optional>

/**
 * @brief Task协程类模板
 * 
 * Task代表一个异步计算任务，可以返回一个值或抛出异常。
 * 它是异步编程中的基础构建块，类似于JavaScript的Promise或C#的Task。
 * 
 * 核心特性：
 * 1. 支持异步计算和返回值
 * 2. 支持异常传播
 * 3. 支持链式调用（通过co_await）
 * 4. 延迟执行（lazy evaluation）
 */
template<typename T = void>
class Task {
public:
    /**
     * @brief Promise类型 - 定义Task协程的行为
     * 
     * 与Generator不同，Task的Promise需要处理：
     * - 返回值的存储
     * - 异常的传播
     * - 等待者的通知
     */
    struct promise_type {
        // 使用variant来存储值或异常
        std::variant<std::monostate, T, std::exception_ptr> result_;
        std::coroutine_handle<> continuation_;  // 等待这个Task的协程
        
        /**
         * @brief get_return_object() - 创建Task对象
         */
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        /**
         * @brief initial_suspend() - 初始挂起策略
         * 
         * Task使用suspend_always实现延迟执行，
         * 只有当被co_await时才开始执行
         */
        std::suspend_always initial_suspend() { return {}; }
        
        /**
         * @brief final_suspend() - 结束时的挂起策略
         * 
         * 返回一个自定义的awaitable，用于通知等待者
         */
        auto final_suspend() noexcept {
            struct FinalAwaiter {
                std::coroutine_handle<> continuation;
                
                bool await_ready() noexcept { return false; }
                
                /**
                 * @brief await_suspend() - 在最终挂起时调用
                 * 
                 * 如果有协程在等待当前Task，则恢复等待者的执行
                 */
                std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type>) noexcept {
                    if (continuation) {
                        return continuation;  // 恢复等待者
                    }
                    return std::noop_coroutine();  // 没有等待者，返回noop
                }
                
                void await_resume() noexcept {}
            };
            
            return FinalAwaiter{continuation_};
        }
        
        /**
         * @brief return_value() - 处理co_return语句
         * 
         * 当Task完成并返回值时调用
         */
        void return_value(const T& value) {
            result_ = value;
        }
        
        /**
         * @brief return_value() - 处理co_return语句（移动版本）
         */
        void return_value(T&& value) {
            result_ = std::move(value);
        }
        
        /**
         * @brief unhandled_exception() - 处理未捕获异常
         * 
         * 将异常存储起来，稍后在await_resume中重新抛出
         */
        void unhandled_exception() {
            result_ = std::current_exception();
        }
        
        /**
         * @brief 获取结果
         * 
         * 如果是异常则重新抛出，否则返回值
         */
        T get_result() {
            if (std::holds_alternative<std::exception_ptr>(result_)) {
                std::rethrow_exception(std::get<std::exception_ptr>(result_));
            }
            return std::get<T>(result_);
        }
    };
    
    /**
     * @brief Task的Awaiter - 使Task可以被co_await
     * 
     * 这个结构体定义了当一个Task被co_await时的行为
     */
    struct TaskAwaiter {
        std::coroutine_handle<promise_type> coro_;
        
        /**
         * @brief await_ready() - 检查是否已经准备好
         * 
         * 如果Task已经完成，返回true，否则返回false
         */
        bool await_ready() const noexcept {
            return coro_.done();
        }
        
        /**
         * @brief await_suspend() - 挂起当前协程
         * 
         * 设置continuation并启动Task的执行
         * 
         * @param awaiting_coro 等待Task的协程
         */
        void await_suspend(std::coroutine_handle<> awaiting_coro) {
            coro_.promise().continuation_ = awaiting_coro;
            coro_.resume();  // 开始执行Task
        }
        
        /**
         * @brief await_resume() - 获取Task的结果
         * 
         * 当Task完成时，这个函数被调用来获取结果
         */
        T await_resume() {
            return coro_.promise().get_result();
        }
    };
    
    /**
     * @brief 构造函数
     */
    explicit Task(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    /**
     * @brief 析构函数
     */
    ~Task() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    /**
     * @brief 移动构造函数
     */
    Task(Task&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    /**
     * @brief 移动赋值操作符
     */
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    // 禁用拷贝
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    
    /**
     * @brief co_await操作符 - 使Task可以被等待
     */
    TaskAwaiter operator co_await() {
        return TaskAwaiter{handle_};
    }
    
    /**
     * @brief 检查Task是否完成
     */
    bool done() const {
        return handle_.done();
    }
    
    /**
     * @brief 手动启动Task执行（如果需要）
     */
    void resume() {
        if (handle_ && !handle_.done()) {
            handle_.resume();
        }
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

/**
 * @brief Task<void>的特化版本
 * 
 * 对于不返回值的异步任务，我们需要特化Task模板
 */
template<>
class Task<void> {
public:
    struct promise_type {
        std::variant<std::monostate, std::exception_ptr> result_;
        std::coroutine_handle<> continuation_;
        
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        
        auto final_suspend() noexcept {
            struct FinalAwaiter {
                std::coroutine_handle<> continuation;
                
                bool await_ready() noexcept { return false; }
                
                std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type>) noexcept {
                    if (continuation) {
                        return continuation;
                    }
                    return std::noop_coroutine();
                }
                
                void await_resume() noexcept {}
            };
            
            return FinalAwaiter{continuation_};
        }
        
        /**
         * @brief return_void() - 处理无返回值的co_return
         */
        void return_void() {
            // 对于void Task，我们只需要标记完成即可
        }
        
        void unhandled_exception() {
            result_ = std::current_exception();
        }
        
        void get_result() {
            if (std::holds_alternative<std::exception_ptr>(result_)) {
                std::rethrow_exception(std::get<std::exception_ptr>(result_));
            }
        }
    };
    
    struct TaskAwaiter {
        std::coroutine_handle<promise_type> coro_;
        
        bool await_ready() const noexcept {
            return coro_.done();
        }
        
        void await_suspend(std::coroutine_handle<> awaiting_coro) {
            coro_.promise().continuation_ = awaiting_coro;
            coro_.resume();
        }
        
        void await_resume() {
            coro_.promise().get_result();
        }
    };
    
    explicit Task(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    ~Task() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    Task(Task&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    
    TaskAwaiter operator co_await() {
        return TaskAwaiter{handle_};
    }
    
    bool done() const {
        return handle_.done();
    }
    
    void resume() {
        if (handle_ && !handle_.done()) {
            handle_.resume();
        }
    }

private:
    std::coroutine_handle<promise_type> handle_;
}; 