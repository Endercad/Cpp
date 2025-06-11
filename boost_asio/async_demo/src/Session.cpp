#include "Session.h"
#include <iostream>
#include <functional>
#include <cstring>

using namespace std;

/**
 * 启动会话 - 异步IO链条的起点
 * 
 * 工作流程：
 * 1. 清空数据缓冲区
 * 2. 发起异步读取操作
 * 3. 注册回调函数HandleRead
 * 4. 函数立即返回，不阻塞
 * 5. 当有数据到达时，io_context会调用HandleRead
 */
void Session::Start(){
    // 清空数据缓冲区，准备接收新数据
    memset(_data, 0, max_length);
    
    // 🔥 关键：异步读取操作
    // - async_read_some: 异步读取一些数据（不一定填满缓冲区）
    // - boost::asio::buffer: 创建缓冲区包装器
    // - std::bind: 绑定回调函数和参数
    // - placeholders: 占位符，在回调时会被实际参数替换
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
        std::bind(&Session::HandleRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    
    // 注意：这里函数立即返回，读取操作在后台进行
    // 当数据到达时，io_context会自动调用HandleRead回调
}

/**
 * 异步读取完成的回调函数
 * 
 * 异步读写循环的核心逻辑：
 * 读取成功 -> 处理数据 -> 异步写入 -> 写入完成 -> 继续读取
 * 
 * @param error 错误码，成功时为空
 * @param bytes_transferred 实际读取的字节数
 */
void Session::HandleRead(const boost::system::error_code& error, size_t bytes_transferred){
    if(!error){
        // 🎯 数据处理：显示接收到的数据
        cout << "Received data: " << string(_data, bytes_transferred) << endl;
        
        // 🔥 关键：异步写入操作（回显服务器）
        // - async_write: 异步写入所有数据（与async_read_some不同）
        // - 将接收到的数据原样发送回客户端
        // - 注册HandleWrite回调函数
        boost::asio::async_write(_socket, boost::asio::buffer(_data, bytes_transferred),
            std::bind(&Session::HandleWrite, this, boost::asio::placeholders::error));
        
        // 注意：这里也是立即返回，写入操作在后台进行
        // 写入完成后会调用HandleWrite回调
    }
    else{
        // 读取出错：显示错误信息并关闭连接
        cout << "Error in HandleRead: " << error.message() << endl;
        _socket.close();
        // 注意：这里应该删除Session对象，但当前代码有内存泄漏问题
    }
}

/**
 * 异步写入完成的回调函数
 * 
 * 写入完成后继续读取，形成读->写->读的循环
 * 这样可以持续处理客户端的请求
 * 
 * @param error 错误码，成功时为空
 */
void Session::HandleWrite(const boost::system::error_code& error){
    if(!error){
        // 写入成功：准备接收下一批数据
        
        // 清空缓冲区，准备下次读取
        memset(_data, 0, max_length);
        
        // 🔄 关键：继续异步读取，形成读写循环
        // 这是异步IO的精髓：完成一个操作后立即发起下一个操作
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
        std::bind(&Session::HandleRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        
        // 这样就形成了完整的异步读写循环：
        // Start() -> HandleRead() -> HandleWrite() -> HandleRead() -> ...
    }
    else{
        // 写入出错：显示错误信息并关闭连接
        cout << "Error in HandleWrite: " << error.message() << endl;
        _socket.close();
    }
}

/**
 * 服务器构造函数
 * 
 * 初始化过程：
 * 1. 创建TCP接受器，绑定到指定端口
 * 2. 开始监听连接
 * 3. 启动异步接受循环
 * 
 * @param io_context 事件循环管理器
 * @param port 监听端口
 */
Server::Server(boost::asio::io_context& io_context, short port)
    :_io_context(io_context), 
     _acceptor(io_context, tcp::endpoint(tcp::v4(), port)){  // 创建acceptor并绑定端口
    
    cout << "Server started on port " << port << endl;
    
    // 🚀 启动异步接受循环
    start_accept();
}

/**
 * 开始接受新连接
 * 
 * 异步接受的工作流程：
 * 1. 为新连接创建Session对象
 * 2. 发起异步接受操作
 * 3. 注册handle_accept回调
 * 4. 函数立即返回，继续处理其他事件
 * 5. 当有连接到达时，调用handle_accept
 */
void Server::start_accept(){
    // 🏗️ 为新连接创建Session对象
    // 注意：每个连接都有自己独立的Session对象
    Session* new_session = new Session(_io_context);
    
    // 🔥 关键：异步接受连接
    // - async_accept: 异步接受一个新连接
    // - new_session->Socket(): 获取新Session的socket
    // - 绑定handle_accept回调函数
    _acceptor.async_accept(new_session->Socket(),
    std::bind(&Server::handle_accept, this, new_session, boost::asio::placeholders::error));
    
    // ！！！函数立即返回，接受操作在后台进行
    // 这样服务器可以同时处理现有连接和等待新连接
}

/**
 * 处理新连接的回调函数
 * 
 * 多连接并发处理的核心：
 * 1. 启动新连接的数据处理
 * 2. 立即准备接受下一个连接
 * 3. 这样可以同时处理多个连接
 * 
 * @param new_session 新连接对应的Session对象
 * @param error 接受连接时的错误码
 */
void Server::handle_accept(Session* new_session, const boost::system::error_code& error){
    if(!error){
        // 连接成功：启动新连接的数据处理
        // 🎯 开始异步读写循环
        new_session->Start();   //Start函数注册异步读取，会立即返回，不会阻塞
        
        cout << "New client connected!" << endl;
    }
    else{
        // 连接失败：清理资源
        cout << "Error in handle_accept: " << error.message() << endl;
        delete new_session;
    }
    
    // 🔄 关键：立即准备接受下一个连接
    // 这是实现多连接并发的关键：
    // - 处理完一个连接后立即监听下一个
    // - 所有连接在同一个io_context中并发处理
    // - 不会因为处理一个连接而阻塞其他连接
    start_accept();
}

/*
 * 异步IO工作流程总结：
 * 
 * 1. 服务器启动流程：
 *    main() -> Server构造 -> start_accept() -> 等待连接
 * 
 * 2. 新连接处理流程：
 *    连接到达 -> handle_accept() -> Session::Start() -> async_read_some()
 * 
 * 3. 数据处理循环：
 *    数据到达 -> HandleRead() -> async_write() -> HandleWrite() -> async_read_some()
 * 
 * 4. 多连接并发：
 *    - 每个连接有独立的Session对象
 *    - 所有异步操作注册到同一个io_context
 *    - io_context.run()在单线程中处理所有事件
 *    - 通过事件驱动实现高并发
 * 
 * 5. 关键特点：
 *    - 非阻塞：所有IO操作立即返回
 *    - 事件驱动：通过回调函数处理IO完成事件
 *    - 高并发：单线程处理多个连接
 *    - 高效：避免线程切换开销
 */