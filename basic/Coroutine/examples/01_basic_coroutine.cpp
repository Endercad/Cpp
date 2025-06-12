/**
 * @file 01_basic_coroutine.cpp
 * @brief C++20协程基础概念示例
 * 
 * 这个文件包含了C++20协程的最基本概念和用法，包括：
 * 1. 协程的三个关键字：co_yield, co_return, co_await
 * 2. Promise类型和协程句柄的基本使用
 * 3. 协程的生命周期和状态管理
 * 4. 简单的协程实现示例
 */

#include <iostream>
#include <coroutine>
#include <string>
#include <utility>

/**
 * @brief 最简单的协程返回类型
 * 
 * 为了创建一个协程函数，我们需要定义一个包含 promise_type 的返回类型。
 * 这个例子展示了一个最基础的协程类型实现。
 */
struct SimpleCoroutine {
    /**
     * @brief Promise类型 - 协程的核心控制器
     * 
     * Promise类型定义了协程的行为，包括：
     * - 如何创建协程返回对象
     * - 协程的初始和最终挂起策略
     * - 如何处理返回值和异常
     */
    struct promise_type {
        /**
         * @brief 创建协程返回对象
         * 
         * 当协程函数被调用时，首先会调用这个函数来创建返回对象
         */
        SimpleCoroutine get_return_object() {
            return SimpleCoroutine{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        
        /**
         * @brief 初始挂起策略
         * 
         * suspend_never: 协程立即开始执行
         * suspend_always: 协程创建后立即挂起，需要手动恢复
         */
        std::suspend_never initial_suspend() { 
            std::cout << "  [Promise] 协程开始执行\n";
            return {}; 
        }
        
        /**
         * @brief 最终挂起策略
         * 
         * 当协程即将结束时调用
         */
        std::suspend_never final_suspend() noexcept { 
            std::cout << "  [Promise] 协程即将结束\n";
            return {}; 
        }
        
        /**
         * @brief 处理 co_return 语句（无返回值）
         */
        void return_void() {
            std::cout << "  [Promise] 协程返回（无值）\n";
        }
        
        /**
         * @brief 处理未捕获的异常
         */
        void unhandled_exception() {
            std::cout << "  [Promise] 协程发生未捕获异常\n";
            std::terminate();
        }
    };
    
    /**
     * @brief 协程句柄 - 用于控制协程的执行
     */
    std::coroutine_handle<promise_type> handle_;
    
    /**
     * @brief 构造函数
     */
    explicit SimpleCoroutine(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    /**
     * @brief 析构函数 - 清理协程资源
     */
    ~SimpleCoroutine() {
        if (handle_) {
            std::cout << "  [协程对象] 销毁协程句柄\n";
            handle_.destroy();
        }
    }
    
    /**
     * @brief 移动构造函数
     */
    SimpleCoroutine(SimpleCoroutine&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    /**
     * @brief 移动赋值操作符
     */
    SimpleCoroutine& operator=(SimpleCoroutine&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    // 禁用拷贝构造和拷贝赋值
    SimpleCoroutine(const SimpleCoroutine&) = delete;
    SimpleCoroutine& operator=(const SimpleCoroutine&) = delete;
};

/**
 * @brief 一个简单的协程函数
 * 
 * 这个函数演示了协程的基本结构和执行流程
 */
SimpleCoroutine simple_coroutine_example() {
    std::cout << "1. 协程函数开始执行\n";
    
    std::cout << "2. 协程函数执行中...\n";
    
    std::cout << "3. 协程函数即将结束\n";
    
    // co_return 语句标志着协程的结束
    co_return;
}

/**
 * @brief 带有yield功能的协程类型
 * 
 * 这个例子展示了如何实现支持 co_yield 的协程
 */
struct YieldCoroutine {
    struct promise_type {
        std::string current_value;  // 存储当前yield的值
        
        YieldCoroutine get_return_object() {
            return YieldCoroutine{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        
        /**
         * @brief 使用 suspend_always 让协程在创建后立即挂起
         * 
         * 这样我们可以手动控制协程的执行
         */
        std::suspend_always initial_suspend() { 
            std::cout << "  [YieldPromise] 协程创建后立即挂起\n";
            return {}; 
        }
        
        std::suspend_always final_suspend() noexcept { 
            std::cout << "  [YieldPromise] 协程在结束时挂起\n";
            return {}; 
        }
        
        /**
         * @brief 处理 co_yield 表达式
         * 
         * 当协程中使用 co_yield 时，这个函数会被调用
         * 
         * @param value yield的值
         * @return 挂起对象，决定是否挂起协程
         */
        std::suspend_always yield_value(const std::string& value) {
            std::cout << "  [YieldPromise] yield值: " << value << "\n";
            current_value = value;
            return {};  // 挂起协程
        }
        
        void return_void() {
            std::cout << "  [YieldPromise] 协程正常结束\n";
        }
        
        void unhandled_exception() {
            std::terminate();
        }
    };
    
    std::coroutine_handle<promise_type> handle_;
    
    explicit YieldCoroutine(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    ~YieldCoroutine() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    YieldCoroutine(YieldCoroutine&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    YieldCoroutine& operator=(YieldCoroutine&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    YieldCoroutine(const YieldCoroutine&) = delete;
    YieldCoroutine& operator=(const YieldCoroutine&) = delete;
    
    /**
     * @brief 恢复协程执行
     * 
     * @return 如果协程还未完成则返回true
     */
    bool resume() {
        if (handle_ && !handle_.done()) {
            std::cout << "  [协程控制] 恢复协程执行\n";
            handle_.resume();
            return !handle_.done();
        }
        return false;
    }
    
    /**
     * @brief 检查协程是否完成
     */
    bool done() const {
        return handle_.done();
    }
    
    /**
     * @brief 获取当前yield的值
     */
    std::string current_value() const {
        if (handle_) {
            return handle_.promise().current_value;
        }
        return "";
    }
};

/**
 * @brief 使用 co_yield 的协程函数
 */
YieldCoroutine yield_example() {
    std::cout << "协程开始执行\n";
    
    co_yield "第一个值";
    std::cout << "第一次yield后继续执行\n";
    
    co_yield "第二个值";
    std::cout << "第二次yield后继续执行\n";
    
    co_yield "第三个值";
    std::cout << "第三次yield后继续执行\n";
    
    std::cout << "协程即将结束\n";
    co_return;
}

/**
 * @brief 演示协程的基本概念
 */
void demonstrate_basic_concepts() {
    std::cout << "\n=== C++20 协程基本概念演示 ===\n\n";
    
    // 协程的关键概念：
    std::cout << "📚 协程的核心概念：\n";
    std::cout << "1. co_yield  - 暂停协程并产生一个值\n";
    std::cout << "2. co_return - 结束协程并可选地返回一个值\n";
    std::cout << "3. co_await  - 等待另一个异步操作完成\n\n";
    
    std::cout << "📚 协程的核心组件：\n";
    std::cout << "1. Promise Type  - 定义协程的行为和状态\n";
    std::cout << "2. Coroutine Handle - 控制协程的执行（恢复、销毁等）\n";
    std::cout << "3. Awaitable Objects - 可以被co_await的对象\n\n";
}

/**
 * @brief 演示简单协程的使用
 */
void demonstrate_simple_coroutine() {
    std::cout << "=== 1. 简单协程示例 ===\n";
    std::cout << "调用协程函数...\n";
    
    // 调用协程函数会立即创建协程对象
    auto coro = simple_coroutine_example();
    
    std::cout << "协程函数调用完成\n";
    std::cout << "协程对象即将销毁...\n\n";
    
    // 当 coro 离开作用域时，协程对象会被销毁
}

/**
 * @brief 演示yield协程的使用
 */
void demonstrate_yield_coroutine() {
    std::cout << "=== 2. Yield协程示例 ===\n";
    std::cout << "创建yield协程...\n";
    
    auto coro = yield_example();
    
    std::cout << "协程创建完成，现在手动控制执行：\n\n";
    
    // 手动控制协程的执行
    int step = 1;
    while (!coro.done()) {
        std::cout << "--- 步骤 " << step++ << " ---\n";
        
        bool has_more = coro.resume();
        
        if (!coro.done()) {
            std::cout << "协程yield的值: \"" << coro.current_value() << "\"\n";
        }
        
        if (!has_more) {
            std::cout << "协程执行完成\n";
            break;
        }
        
        std::cout << "协程已挂起，等待下次恢复...\n\n";
    }
    
    std::cout << "yield协程示例完成\n\n";
}

/**
 * @brief 演示协程的生命周期
 */
void demonstrate_coroutine_lifecycle() {
    std::cout << "=== 3. 协程生命周期演示 ===\n\n";
    
    std::cout << "协程生命周期的关键阶段：\n";
    std::cout << "1. 创建阶段：调用协程函数，创建promise对象和协程句柄\n";
    std::cout << "2. 初始挂起：根据initial_suspend()的返回值决定是否立即执行\n";
    std::cout << "3. 执行阶段：协程体代码执行，可能包含yield/await点\n";
    std::cout << "4. 最终挂起：根据final_suspend()的返回值决定如何结束\n";
    std::cout << "5. 销毁阶段：清理协程资源\n\n";
    
    std::cout << "创建一个协程来观察生命周期：\n";
    {
        auto coro = yield_example();
        std::cout << "协程对象在作用域内...\n";
        coro.resume();  // 执行到第一个yield点
        std::cout << "协程在第一个yield点挂起\n";
        // 当离开作用域时，协程对象会被销毁
    }
    std::cout << "协程对象已离开作用域并被销毁\n\n";
}

/**
 * @brief 主函数
 */
int main() {
    try {
        std::cout << "🚀 C++20 协程基础概念学习\n";
        std::cout << "========================================\n";
        
        // 演示基本概念
        demonstrate_basic_concepts();
        
        // 演示简单协程
        demonstrate_simple_coroutine();
        
        // 演示yield协程
        demonstrate_yield_coroutine();
        
        // 演示协程生命周期
        demonstrate_coroutine_lifecycle();
        
        std::cout << "✅ 基础协程示例完成！\n";
        std::cout << "\n📖 学习要点总结：\n";
        std::cout << "1. 协程函数必须返回包含promise_type的类型\n";
        std::cout << "2. Promise类型定义了协程的行为和状态管理\n";
        std::cout << "3. 协程句柄用于控制协程的执行（恢复、暂停、销毁）\n";
        std::cout << "4. co_yield用于暂停协程并产生值\n";
        std::cout << "5. co_return用于结束协程\n";
        std::cout << "6. 协程的生命周期由promise_type的各个方法控制\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}