#ifndef SESSION_H
#define SESSION_H

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

/**
 * Session类 - 管理单个客户端连接的会话
 * 
 * 异步IO工作原理：
 * 1. 每个客户端连接对应一个Session对象
 * 2. 使用异步读写操作，不会阻塞线程
 * 3. 通过回调函数处理IO完成事件
 * 4. 所有操作都在io_context的事件循环中执行
 */
class Session{
public:
    /**
     * 构造函数 - 初始化Session
     * @param io_context 事件循环管理器，所有异步操作都注册到这里
     * 
     * 关键点：socket必须与io_context关联，这样：
     * - socket的异步操作可以被io_context调度
     * - 事件完成后可以触发对应的回调函数
     */
    Session(boost::asio::io_context& io_context):_socket(io_context){
        
    }
    
    /**
     * 获取socket引用 - 用于Server类接受连接
     * @return tcp::socket引用，Server用它来接受新连接
     */
    tcp::socket& Socket(){return _socket;}

    /**
     * 启动会话 - 开始异步读取数据
     * 这是异步IO链条的起点
     */
    void Start();
    
private:
    /**
     * 异步读取完成的回调函数
     * 
     * 异步读写流程的核心：
     * 读取完成 -> 处理数据 -> 异步写入 -> 写入完成 -> 继续读取
     * 
     * @param error 错误码，如果操作成功则为空
     * @param bytes_transferred 实际读取的字节数
     */
    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);
    
    /**
     * 异步写入完成的回调函数
     * 
     * 写入完成后继续监听新的数据
     * 
     * @param error 错误码，如果操作成功则为空
     */
    void HandleWrite(const boost::system::error_code& error);

    tcp::socket _socket;           // TCP套接字，与单个客户端的连接
    enum {max_length = 1024};      // 缓冲区最大长度（编译时常量）
    char _data[max_length];        // 数据缓冲区，存储读取/发送的数据
};

/**
 * Server类 - TCP服务器，负责接受客户端连接
 * 
 * 多连接处理原理：
 * 1. 使用async_accept异步接受连接，不阻塞
 * 2. 每个新连接创建一个Session对象
 * 3. 接受一个连接后立即准备接受下一个连接
 * 4. 所有连接在同一个io_context中并发处理
 */
class Server{
public:
    /**
     * 构造函数 - 初始化服务器
     * @param io_context 事件循环管理器
     * @param port 监听端口
     */
    Server(boost::asio::io_context& io_context, short port);
    
private:
    /**
     * 开始接受新连接
     * 
     * 异步接受流程：
     * 1. 创建新的Session对象
     * 2. 调用async_accept开始监听
     * 3. 当有连接到达时，触发handle_accept回调
     */
    void start_accept();
    
    /**
     * 处理新连接的回调函数
     * 
     * 多连接并发的关键：
     * 1. 启动新连接的Session
     * 2. 立即调用start_accept()准备接受下一个连接
     * 3. 这样可以同时处理多个连接
     * 
     * @param new_session 新连接对应的Session对象
     * @param error 接受连接时的错误码
     */
    void handle_accept(Session* new_session, const boost::system::error_code& error);
    
    boost::asio::io_context& _io_context;  // 事件循环管理器引用
    tcp::acceptor _acceptor;               // TCP接受器，监听指定端口的连接
};

#endif