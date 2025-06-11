/**
 * Boost.Asio 异步TCP服务器 - 主程序
 * 
 * 这是一个异步回显服务器，演示了：
 * 1. 异步IO的基本概念和使用方法
 * 2. 单线程处理多个并发连接
 * 3. 事件驱动的编程模式
 * 4. 非阻塞IO操作
 * 
 * 程序架构：
 * - io_context: 事件循环管理器，是整个异步系统的心脏
 * - Server: 负责接受新连接
 * - Session: 负责处理单个客户端连接的数据读写
 */

#include <iostream>
#include "../include/Session.h"

using namespace std;

/**
 * 程序入口函数
 * 
 * 异步服务器的启动流程：
 * 1. 创建io_context事件循环管理器
 * 2. 创建Server对象，开始监听端口
 * 3. 运行io_context事件循环
 * 4. 处理所有异步事件（连接、读取、写入）
 */
int main(){
    try{
        // 🎯 创建事件循环管理器
        // io_context是Boost.Asio的核心，负责：
        // - 管理所有异步操作
        // - 调度回调函数的执行
        // - 与操作系统的IO多路复用机制交互（Linux上是epoll）
        boost::asio::io_context io_context;
        
        // 🏗️ 创建服务器对象
        // - 监听8848端口
        // - 自动开始接受连接
        // - 所有异步操作都注册到io_context中
        Server server(io_context, 8848);
        cout << "Server is running on port 8848..." << endl;
        cout << "可以使用 telnet localhost 8848 进行测试" << endl;
        cout << "或者使用多个telnet客户端测试并发连接" << endl;
        
        // 🔥 关键：启动事件循环
        // io_context.run()是整个异步系统的驱动引擎：
        // 
        // 工作原理：
        // 1. 检查是否有就绪的异步操作
        // 2. 如果有，调用对应的回调函数
        // 3. 如果没有，等待操作系统通知（通过epoll等机制）
        // 4. 循环执行步骤1-3，直到没有更多异步操作
        // 
        // 在单线程中处理：
        // - 新连接的接受 (handle_accept)
        // - 数据的读取 (HandleRead)  
        // - 数据的写入 (HandleWrite)
        // - 所有客户端的并发请求
        io_context.run();
        
        // 注意：run()会阻塞在这里，直到：
        // 1. 所有异步操作完成且没有新操作
        // 2. 显式调用io_context.stop()
        // 3. 发生未捕获的异常
        
    }
    catch(exception& e){
        // 捕获并显示任何异常
        cout << "服务器异常: " << e.what() << endl;
    }
    
    cout << "服务器已退出" << endl;
    return 0;
}

/*
 * 🚀 异步服务器完整工作流程：
 * 
 * 1. 程序启动：
 *    main() -> 创建io_context -> 创建Server -> io_context.run()
 * 
 * 2. 等待连接：
 *    Server构造函数 -> start_accept() -> async_accept() -> 等待客户端连接
 * 
 * 3. 客户端连接：
 *    连接到达 -> handle_accept()回调 -> 创建Session -> Session::Start()
 *    同时 -> start_accept() 准备接受下一个连接
 * 
 * 4. 数据处理循环：
 *    客户端发送数据 -> HandleRead()回调 -> 显示数据 -> async_write()
 *    写入完成 -> HandleWrite()回调 -> async_read_some() -> 等待更多数据
 * 
 * 5. 多客户端并发：
 *    - 每个客户端有独立的Session对象
 *    - 所有Session的异步操作都在同一个io_context中
 *    - 单线程通过事件驱动处理所有连接
 *    - 高效且无线程竞争问题
 * 
 * 💡 关键概念理解：
 * 
 * - 异步 vs 同步：
 *   同步：调用函数 -> 等待完成 -> 返回结果 （阻塞）
 *   异步：调用函数 -> 立即返回 -> 完成时调用回调 （非阻塞）
 * 
 * - 单线程高并发：
 *   不需要为每个连接创建线程
 *   通过事件循环在单线程中处理多个连接
 *   避免线程切换和同步开销
 * 
 * - 事件驱动：
 *   程序不主动轮询状态
 *   等待系统通知事件发生
 *   响应式处理，高效节能
 */