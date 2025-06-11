#ifndef IMPROVED_SESSION_H
#define IMPROVED_SESSION_H

#include <iostream>
#include <boost/asio.hpp>
#include <queue>
#include <string>
#include <memory>

using boost::asio::ip::tcp;

/**
 * 改进的Session类 - 实现真正的全双工通信
 * 
 * 核心改进：
 * 1. 收发分离：读取和写入独立进行
 * 2. 发送队列：保证发送顺序
 * 3. 线程安全：避免多个async_write重叠
 * 4. 全双工：可以同时读取和发送数据
 */
class ImprovedSession : public std::enable_shared_from_this<ImprovedSession> {
public:
    /**
     * 构造函数
     * @param io_context 事件循环管理器
     */
    ImprovedSession(boost::asio::io_context& io_context);
    
    /**
     * 获取socket引用
     * @return tcp::socket引用
     */
    tcp::socket& Socket() { return _socket; }
    
    /**
     * 启动会话 - 开始异步读取
     */
    void Start();
    
    /**
     * 发送消息（线程安全）
     * @param message 要发送的消息
     */
    void SendMessage(const std::string& message);
    
private:
    /**
     * 异步读取完成回调
     * @param error 错误码
     * @param bytes_transferred 读取的字节数
     */
    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);
    
    /**
     * 执行写入操作
     * 从发送队列中取出数据进行发送
     */
    void DoWrite();
    
    /**
     * 异步写入完成回调
     * @param error 错误码
     */
    void HandleWrite(const boost::system::error_code& error);
    
    tcp::socket _socket;                    // TCP套接字
    enum { max_length = 1024 };             // 缓冲区大小
    char _read_buffer[max_length];          // 读取缓冲区
    
    std::queue<std::string> _write_queue;   // 发送队列
    bool _writing;                          // 是否正在写入（防止重叠）
};

/**
 * 改进的Server类
 */
class ImprovedServer {
public:
    /**
     * 构造函数
     * @param io_context 事件循环管理器
     * @param port 监听端口
     */
    ImprovedServer(boost::asio::io_context& io_context, short port);
    
private:
    /**
     * 开始接受新连接
     */
    void StartAccept();
    
    /**
     * 处理新连接回调
     * @param new_session 新连接会话
     * @param error 错误码
     */
    void HandleAccept(std::shared_ptr<ImprovedSession> new_session, 
                     const boost::system::error_code& error);
    
    boost::asio::io_context& _io_context;  // 事件循环管理器
    tcp::acceptor _acceptor;               // TCP接受器
};

#endif 