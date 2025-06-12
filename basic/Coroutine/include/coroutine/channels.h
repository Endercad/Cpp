#pragma once

#include <coroutine>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

/**
 * @brief 协程通道 - 用于协程间通信
 * 
 * Channel是协程之间进行数据传输的机制，类似于Go语言的channel。
 * 它支持多个生产者和多个消费者，并且是线程安全的。
 * 
 * 核心概念：
 * 1. 发送操作：将数据发送到通道中
 * 2. 接收操作：从通道中接收数据
 * 3. 阻塞语义：当通道满时发送会阻塞，当通道空时接收会阻塞
 */
template<typename T>
class Channel {
public:
    /**
     * @brief 构造函数
     * @param capacity 通道容量，0表示无缓冲通道
     */
    explicit Channel(size_t capacity = 0) : capacity_(capacity), closed_(false) {}
    
    /**
     * @brief 发送操作的awaitable
     * 
     * 这个类实现了发送数据到通道的异步操作
     */
    class SendAwaiter {
    public:
        SendAwaiter(Channel& channel, T value) 
            : channel_(channel), value_(std::move(value)) {}
        
        /**
         * @brief await_ready() - 检查是否可以立即发送
         * 
         * 如果通道有空间或者有接收者在等待，则可以立即发送
         */
        bool await_ready() const noexcept {
            std::lock_guard<std::mutex> lock(channel_.mutex_);
            return !channel_.closed_ && 
                   (channel_.buffer_.size() < channel_.capacity_ || 
                    !channel_.receive_waiters_.empty());
        }
        
        /**
         * @brief await_suspend() - 挂起发送者
         * 
         * 如果不能立即发送，则将发送者加入等待队列
         */
        void await_suspend(std::coroutine_handle<> handle) {
            std::lock_guard<std::mutex> lock(channel_.mutex_);
            
            if (channel_.closed_) {
                throw std::runtime_error("通道已关闭，无法发送数据");
            }
            
            // 检查是否有接收者在等待
            if (!channel_.receive_waiters_.empty()) {
                auto receiver = channel_.receive_waiters_.front();
                channel_.receive_waiters_.pop();
                
                // 直接将数据传递给接收者
                receiver_result_ = std::move(value_);
                receiver.resume();
                return;  // 不需要挂起
            }
            
            // 检查缓冲区是否有空间
            if (channel_.buffer_.size() < channel_.capacity_) {
                channel_.buffer_.push(std::move(value_));
                return;  // 不需要挂起
            }
            
            // 需要等待，加入发送者队列
            channel_.send_waiters_.push({handle, std::move(value_)});
        }
        
        /**
         * @brief await_resume() - 发送完成后的处理
         */
        void await_resume() {
            if (channel_.closed_) {
                throw std::runtime_error("通道已关闭，发送失败");
            }
        }
        
    private:
        Channel& channel_;
        T value_;
        mutable std::optional<T> receiver_result_;
    };
    
    /**
     * @brief 接收操作的awaitable
     * 
     * 这个类实现了从通道接收数据的异步操作
     */
    class ReceiveAwaiter {
    public:
        explicit ReceiveAwaiter(Channel& channel) : channel_(channel) {}
        
        /**
         * @brief await_ready() - 检查是否有数据可接收
         */
        bool await_ready() const noexcept {
            std::lock_guard<std::mutex> lock(channel_.mutex_);
            return !channel_.buffer_.empty() || 
                   !channel_.send_waiters_.empty() ||
                   channel_.closed_;
        }
        
        /**
         * @brief await_suspend() - 挂起接收者
         */
        void await_suspend(std::coroutine_handle<> handle) {
            std::lock_guard<std::mutex> lock(channel_.mutex_);
            
            // 检查缓冲区是否有数据
            if (!channel_.buffer_.empty()) {
                result_ = std::move(channel_.buffer_.front());
                channel_.buffer_.pop();
                
                // 唤醒等待的发送者
                if (!channel_.send_waiters_.empty()) {
                    auto sender = channel_.send_waiters_.front();
                    channel_.send_waiters_.pop();
                    
                    channel_.buffer_.push(std::move(sender.value));
                    sender.handle.resume();
                }
                return;  // 不需要挂起
            }
            
            // 检查是否有发送者在等待
            if (!channel_.send_waiters_.empty()) {
                auto sender = channel_.send_waiters_.front();
                channel_.send_waiters_.pop();
                
                result_ = std::move(sender.value);
                sender.handle.resume();
                return;  // 不需要挂起
            }
            
            // 通道为空且已关闭
            if (channel_.closed_) {
                return;  // 不需要挂起，await_resume会抛出异常
            }
            
            // 需要等待，加入接收者队列
            channel_.receive_waiters_.push(handle);
        }
        
        /**
         * @brief await_resume() - 获取接收到的数据
         */
        T await_resume() {
            if (result_.has_value()) {
                return std::move(*result_);
            }
            
            if (channel_.closed_) {
                throw std::runtime_error("通道已关闭，无法接收数据");
            }
            
            // 这种情况不应该发生
            throw std::runtime_error("接收操作异常");
        }
        
    private:
        Channel& channel_;
        std::optional<T> result_;
    };
    
    /**
     * @brief 发送数据到通道
     * 
     * 使用示例：co_await channel.send(42);
     */
    SendAwaiter send(T value) {
        return SendAwaiter{*this, std::move(value)};
    }
    
    /**
     * @brief 从通道接收数据
     * 
     * 使用示例：T value = co_await channel.receive();
     */
    ReceiveAwaiter receive() {
        return ReceiveAwaiter{*this};
    }
    
    /**
     * @brief 关闭通道
     * 
     * 关闭后，所有等待的发送操作会抛出异常，
     * 接收操作会在缓冲区清空后抛出异常
     */
    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) return;
        
        closed_ = true;
        
        // 唤醒所有等待的接收者
        while (!receive_waiters_.empty()) {
            auto receiver = receive_waiters_.front();
            receive_waiters_.pop();
            receiver.resume();
        }
        
        // 唤醒所有等待的发送者（他们会在await_resume中抛出异常）
        while (!send_waiters_.empty()) {
            auto sender = send_waiters_.front();
            send_waiters_.pop();
            sender.handle.resume();
        }
    }
    
    /**
     * @brief 检查通道是否已关闭
     */
    bool is_closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }
    
    /**
     * @brief 获取通道中的数据数量
     */
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.size();
    }
    
    /**
     * @brief 检查通道是否为空
     */
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return buffer_.empty();
    }

private:
    /**
     * @brief 等待发送的项目
     */
    struct SendWaiter {
        std::coroutine_handle<> handle;
        T value;
    };
    
    mutable std::mutex mutex_;                          // 保护通道状态的互斥锁
    std::queue<T> buffer_;                              // 数据缓冲区
    std::queue<SendWaiter> send_waiters_;               // 等待发送的协程队列
    std::queue<std::coroutine_handle<>> receive_waiters_; // 等待接收的协程队列
    size_t capacity_;                                   // 通道容量
    bool closed_;                                       // 是否已关闭
};

/**
 * @brief 创建通道的辅助函数
 * 
 * 使用示例：
 * auto ch = make_channel<int>(10);  // 创建容量为10的int通道
 */
template<typename T>
std::shared_ptr<Channel<T>> make_channel(size_t capacity = 0) {
    return std::make_shared<Channel<T>>(capacity);
} 