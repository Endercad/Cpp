#pragma once

#include <coroutine>
#include <exception>
#include <iterator>
#include <utility>

/**
 * @brief Generator协程类模板
 * 
 * Generator是C++20协程的一个经典应用，它可以生成一系列值并按需计算。
 * 这个实现展示了如何创建一个可迭代的协程对象。
 * 
 * 核心概念：
 * 1. Promise Type（承诺类型）：定义协程的行为
 * 2. Coroutine Handle（协程句柄）：用于控制协程的执行
 * 3. Iterator（迭代器）：使Generator可以在范围for循环中使用
 */
template<typename T>
class Generator {
public:
    /**
     * @brief Promise类型 - 协程的核心
     * 
     * Promise类型定义了协程的行为，包括：
     * - 协程如何开始和结束
     * - 如何处理co_yield和co_return
     * - 如何处理异常
     * - 返回什么类型的对象
     */
    struct promise_type {
        T current_value;  // 当前产生的值
        
        /**
         * @brief get_return_object() - 创建协程返回对象
         * 
         * 这个函数在协程函数被调用时立即执行，
         * 用于创建并返回Generator对象。
         */
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        /**
         * @brief initial_suspend() - 协程初始挂起策略
         * 
         * suspend_always: 协程创建后立即挂起，需要手动恢复
         * suspend_never: 协程创建后立即开始执行
         * 
         * Generator通常使用suspend_always，因为我们希望按需生成值
         */
        std::suspend_always initial_suspend() { return {}; }
        
        /**
         * @brief final_suspend() - 协程结束时的挂起策略
         * 
         * 返回suspend_always确保协程在结束时保持挂起状态，
         * 这样我们就可以安全地检查协程是否完成。
         */
        std::suspend_always final_suspend() noexcept { return {}; }
        
        /**
         * @brief yield_value() - 处理co_yield表达式
         * 
         * 当协程中使用co_yield时，这个函数会被调用。
         * 它保存yielded的值并挂起协程。
         * 
         * @param value 要yield的值
         * @return 挂起对象
         */
        std::suspend_always yield_value(const T& value) {
            current_value = value;
            return {};
        }
        
        /**
         * @brief return_void() - 处理co_return（无返回值）
         * 
         * Generator通常不需要返回最终值，只是结束生成序列
         */
        void return_void() {}
        
        /**
         * @brief unhandled_exception() - 处理未捕获的异常
         * 
         * 如果协程中发生未捕获的异常，这个函数会被调用
         */
        void unhandled_exception() {
            std::terminate();
        }
    };
    
    /**
     * @brief 迭代器类 - 使Generator可以在范围for循环中使用
     * 
     * 这个迭代器实现了输入迭代器的要求，
     * 允许我们像使用容器一样使用Generator。
     */
    struct iterator {
        std::coroutine_handle<promise_type> coro_;  // 协程句柄
        bool done_;  // 是否完成
        
        /**
         * @brief 构造函数
         * @param coro 协程句柄
         */
        explicit iterator(std::coroutine_handle<promise_type> coro) 
            : coro_(coro), done_(!coro || coro.done()) {
            if (!done_) {
                coro_.resume();  // 恢复协程执行
                done_ = coro_.done();  // 检查是否完成
            }
        }
        
        /**
         * @brief 默认构造函数 - 创建end迭代器
         */
        iterator() : coro_(nullptr), done_(true) {}
        
        /**
         * @brief 解引用操作符 - 获取当前值
         */
        const T& operator*() const {
            return coro_.promise().current_value;
        }
        
        /**
         * @brief 前缀递增操作符 - 移动到下一个值
         */
        iterator& operator++() {
            if (coro_ && !coro_.done()) {
                coro_.resume();  // 恢复协程执行
                done_ = coro_.done();  // 检查是否完成
            } else {
                done_ = true;
            }
            return *this;
        }
        
        /**
         * @brief 相等比较操作符
         */
        bool operator==(const iterator& other) const {
            return done_ == other.done_;
        }
        
        /**
         * @brief 不等比较操作符
         */
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
        
        // 迭代器traits
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
    };
    
    /**
     * @brief 构造函数
     * @param handle 协程句柄
     */
    explicit Generator(std::coroutine_handle<promise_type> handle) 
        : handle_(handle) {}
    
    /**
     * @brief 析构函数 - 清理协程资源
     */
    ~Generator() {
        if (handle_) {
            handle_.destroy();
        }
    }
    
    /**
     * @brief 移动构造函数
     */
    Generator(Generator&& other) noexcept 
        : handle_(std::exchange(other.handle_, nullptr)) {}
    
    /**
     * @brief 移动赋值操作符
     */
    Generator& operator=(Generator&& other) noexcept {
        if (this != &other) {
            if (handle_) {
                handle_.destroy();
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }
    
    // 禁用拷贝构造和拷贝赋值
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    
    /**
     * @brief begin() - 返回开始迭代器
     */
    iterator begin() {
        return iterator{handle_};
    }
    
    /**
     * @brief end() - 返回结束迭代器
     */
    iterator end() {
        return iterator{};
    }

private:
    std::coroutine_handle<promise_type> handle_;  // 协程句柄
}; 