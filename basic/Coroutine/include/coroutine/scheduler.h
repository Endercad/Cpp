#pragma once

#include <coroutine>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <atomic>

/**
 * @brief 协程调度器
 * 
 * Scheduler负责管理和调度多个协程的执行。
 * 它提供了一个线程池来执行协程，并支持不同的调度策略。
 * 
 * 核心功能：
 * 1. 协程任务的排队和调度
 * 2. 多线程执行环境
 * 3. 工作窃取（可选）
 * 4. 优先级调度（可选）
 */
class Scheduler {
public:
    /**
     * @brief 调度器任务类型
     * 
     * 封装了协程句柄和相关的调度信息
     */
    struct Task {
        std::coroutine_handle<> handle;  // 协程句柄
        int priority = 0;                // 任务优先级（数值越大优先级越高）
        
        Task(std::coroutine_handle<> h, int p = 0) : handle(h), priority(p) {}
        
        // 用于优先级队列的比较
        bool operator<(const Task& other) const {
            return priority < other.priority;  // 注意：priority_queue是最大堆
        }
    };
    
    /**
     * @brief 构造函数
     * @param thread_count 工作线程数量，0表示使用硬件并发数
     */
    explicit Scheduler(size_t thread_count = 0) 
        : running_(false), stop_requested_(false) {
        if (thread_count == 0) {
            thread_count = std::thread::hardware_concurrency();
            if (thread_count == 0) thread_count = 4;  // 默认4个线程
        }
        thread_count_ = thread_count;
    }
    
    /**
     * @brief 析构函数
     */
    ~Scheduler() {
        stop();
    }
    
    /**
     * @brief 启动调度器
     * 
     * 创建工作线程池并开始处理任务
     */
    void start() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_) return;
        
        running_ = true;
        stop_requested_ = false;
        
        // 创建工作线程
        threads_.reserve(thread_count_);
        for (size_t i = 0; i < thread_count_; ++i) {
            threads_.emplace_back([this, i]() {
                worker_thread(i);
            });
        }
    }
    
    /**
     * @brief 停止调度器
     * 
     * 停止所有工作线程并等待它们完成
     */
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) return;
            
            stop_requested_ = true;
            running_ = false;
        }
        
        // 通知所有工作线程
        condition_.notify_all();
        
        // 等待所有线程完成
        for (auto& thread : threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads_.clear();
    }
    
    /**
     * @brief 调度一个协程
     * @param handle 协程句柄
     * @param priority 任务优先级
     */
    void schedule(std::coroutine_handle<> handle, int priority = 0) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                // 如果调度器未运行，直接执行
                if (handle && !handle.done()) {
                    handle.resume();
                }
                return;
            }
            
            task_queue_.emplace(handle, priority);
        }
        
        condition_.notify_one();
    }
    
    /**
     * @brief 批量调度多个协程
     * @param handles 协程句柄列表
     */
    void schedule_batch(const std::vector<std::coroutine_handle<>>& handles) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                // 如果调度器未运行，直接执行
                for (auto handle : handles) {
                    if (handle && !handle.done()) {
                        handle.resume();
                    }
                }
                return;
            }
            
            for (auto handle : handles) {
                task_queue_.emplace(handle, 0);
            }
        }
        
        condition_.notify_all();
    }
    
    /**
     * @brief 获取当前任务队列大小
     */
    size_t queue_size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return task_queue_.size();
    }
    
    /**
     * @brief 检查调度器是否正在运行
     */
    bool is_running() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return running_;
    }
    
    /**
     * @brief 等待所有任务完成
     * 
     * 这个函数会阻塞直到任务队列为空
     */
    void wait_for_all() {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]() {
            return task_queue_.empty() || !running_;
        });
    }
    
    /**
     * @brief 获取全局调度器实例
     * 
     * 单例模式，确保整个程序只有一个调度器
     */
    static Scheduler& instance() {
        static Scheduler scheduler;
        return scheduler;
    }

private:
    /**
     * @brief 工作线程函数
     * @param thread_id 线程ID
     */
    void worker_thread(size_t thread_id) {
        while (true) {
            Task task{nullptr};
            
            // 获取任务
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this]() {
                    return !task_queue_.empty() || stop_requested_;
                });
                
                if (stop_requested_) {
                    break;
                }
                
                if (!task_queue_.empty()) {
                    task = task_queue_.top();
                    task_queue_.pop();
                }
            }
            
            // 执行任务
            if (task.handle) {
                try {
                    if (!task.handle.done()) {
                        task.handle.resume();
                    }
                } catch (const std::exception& e) {
                    // 记录异常（在实际应用中应该有更好的错误处理）
                    // std::cerr << "协程执行异常: " << e.what() << std::endl;
                } catch (...) {
                    // 处理未知异常
                    // std::cerr << "协程执行未知异常" << std::endl;
                }
            }
        }
    }
    
    mutable std::mutex mutex_;                      // 保护共享状态的互斥锁
    std::condition_variable condition_;             // 用于线程同步的条件变量
    std::priority_queue<Task> task_queue_;          // 任务队列（优先级队列）
    std::vector<std::thread> threads_;              // 工作线程池
    size_t thread_count_;                           // 线程数量
    bool running_;                                  // 是否正在运行
    std::atomic<bool> stop_requested_;              // 是否请求停止
};

/**
 * @brief 可调度的协程任务包装器
 * 
 * 这个类将普通的Task包装成可以自动调度的版本
 */
template<typename T>
class ScheduledTask {
public:
    /**
     * @brief Promise类型 - 支持自动调度
     */
    struct promise_type {
        std::variant<std::monostate, T, std::exception_ptr> result_;
        std::coroutine_handle<> continuation_;
        Scheduler* scheduler_ = nullptr;
        int priority_ = 0;
        
        ScheduledTask get_return_object() {
            return ScheduledTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        
        /**
         * @brief 自定义的final awaiter，支持调度器集成
         */
        auto final_suspend() noexcept {
            struct FinalAwaiter {
                std::coroutine_handle<> continuation;
                Scheduler* scheduler;
                
                bool await_ready() noexcept { return false; }
                
                std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type>) noexcept {
                    if (continuation) {
                        if (scheduler && scheduler->is_running()) {
                            scheduler->schedule(continuation);
                            return std::noop_coroutine();
                        } else {
                            return continuation;
                        }
                    }
                    return std::noop_coroutine();
                }
                
                void await_resume() noexcept {}
            };
            
            return FinalAwaiter{continuation_, scheduler_};
        }
        
        void return_value(const T& value) {
            result_ = value;
        }
        
        void return_value(T&& value) {
            result_ = std::move(value);
        }
        
        void unhandled_exception() {
            result_ = std::current_exception();
        }
        
        T get_result() {
            if (std::holds_alternative<std::exception_ptr>(result_)) {
                std::rethrow_exception(std::get<std::exception_ptr>(result_));
            }
            return std::get<T>(result_);
        }
        
        /**
         * @brief 设置调度器
         */
        void set_scheduler(Scheduler* scheduler, int priority = 0) {
            scheduler_ = scheduler;
            priority_ = priority;
        }
    };
    
    /**
     * @brief Awaiter类型
     */
    struct TaskAwaiter {
        std::coroutine_handle<promise_type> coro_;
        
        bool await_ready() const noexcept {
            return coro_.done();
        }
        
        void await_suspend(std::coroutine_handle<> awaiting_coro) {
            coro_.promise().continuation_ = awaiting_coro;
            
            // 使用调度器启动任务
            if (coro_.promise().scheduler_ && coro_.promise().scheduler_->is_running()) {
                coro_.promise().scheduler_->schedule(coro_, coro_.promise().priority_);
            } else {
                coro_.resume();
            }
        }
        
        T await_resume() {
            return coro_.promise().get_result();
        }
    };
    
    explicit ScheduledTask(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    ~ScheduledTask() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    ScheduledTask(ScheduledTask&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    ScheduledTask& operator=(ScheduledTask&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    ScheduledTask(const ScheduledTask&) = delete;
    ScheduledTask& operator=(const ScheduledTask&) = delete;
    
    TaskAwaiter operator co_await() {
        return TaskAwaiter{handle_};
    }
    
    /**
     * @brief 设置调度器和优先级
     */
    void set_scheduler(Scheduler* scheduler, int priority = 0) {
        if (handle_) {
            handle_.promise().set_scheduler(scheduler, priority);
        }
    }
    
    /**
     * @brief 启动任务（使用指定的调度器）
     */
    void start(Scheduler* scheduler = nullptr, int priority = 0) {
        if (scheduler) {
            set_scheduler(scheduler, priority);
            scheduler->schedule(handle_, priority);
        } else {
            handle_.resume();
        }
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

/**
 * @brief ScheduledTask<void>的特化版本
 */
template<>
class ScheduledTask<void> {
public:
    struct promise_type {
        std::variant<std::monostate, std::exception_ptr> result_;
        std::coroutine_handle<> continuation_;
        Scheduler* scheduler_ = nullptr;
        int priority_ = 0;
        
        ScheduledTask get_return_object() {
            return ScheduledTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        
        auto final_suspend() noexcept {
            struct FinalAwaiter {
                std::coroutine_handle<> continuation;
                Scheduler* scheduler;
                
                bool await_ready() noexcept { return false; }
                
                std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type>) noexcept {
                    if (continuation) {
                        if (scheduler && scheduler->is_running()) {
                            scheduler->schedule(continuation);
                            return std::noop_coroutine();
                        } else {
                            return continuation;
                        }
                    }
                    return std::noop_coroutine();
                }
                
                void await_resume() noexcept {}
            };
            
            return FinalAwaiter{continuation_, scheduler_};
        }
        
        void return_void() {}
        
        void unhandled_exception() {
            result_ = std::current_exception();
        }
        
        void get_result() {
            if (std::holds_alternative<std::exception_ptr>(result_)) {
                std::rethrow_exception(std::get<std::exception_ptr>(result_));
            }
        }
        
        void set_scheduler(Scheduler* scheduler, int priority = 0) {
            scheduler_ = scheduler;
            priority_ = priority;
        }
    };
    
    struct TaskAwaiter {
        std::coroutine_handle<promise_type> coro_;
        
        bool await_ready() const noexcept {
            return coro_.done();
        }
        
        void await_suspend(std::coroutine_handle<> awaiting_coro) {
            coro_.promise().continuation_ = awaiting_coro;
            
            if (coro_.promise().scheduler_ && coro_.promise().scheduler_->is_running()) {
                coro_.promise().scheduler_->schedule(coro_, coro_.promise().priority_);
            } else {
                coro_.resume();
            }
        }
        
        void await_resume() {
            coro_.promise().get_result();
        }
    };
    
    explicit ScheduledTask(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    ~ScheduledTask() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    ScheduledTask(ScheduledTask&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    ScheduledTask& operator=(ScheduledTask&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    ScheduledTask(const ScheduledTask&) = delete;
    ScheduledTask& operator=(const ScheduledTask&) = delete;
    
    TaskAwaiter operator co_await() {
        return TaskAwaiter{handle_};
    }
    
    void set_scheduler(Scheduler* scheduler, int priority = 0) {
        if (handle_) {
            handle_.promise().set_scheduler(scheduler, priority);
        }
    }
    
    void start(Scheduler* scheduler = nullptr, int priority = 0) {
        if (scheduler) {
            set_scheduler(scheduler, priority);
            scheduler->schedule(handle_, priority);
        } else {
            handle_.resume();
        }
    }

private:
    std::coroutine_handle<promise_type> handle_;
}; 