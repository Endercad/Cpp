/**
 * TCP同步客户端 - 与异步服务器通信
 * 
 * 协议格式：[头部(2字节)] + [消息体(变长)]
 * - 头部：2字节，存储后续消息体的长度(short类型)
 * - 消息体：变长，实际的消息内容
 * 
 * 与异步服务器的对比：
 * - 服务器：异步IO，事件驱动，可处理多个并发连接
 * - 客户端：同步IO，阻塞式，简单直观的请求-响应模式
 */

#include <iostream>
#include <boost/asio.hpp>
#include <cstring>

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using namespace std;

const int MAX_LENGTH = 1024;
const int HEAD_LENGTH = 2;

int main()
{
    try {
        /*
         * 🏗️ 第一步：创建网络基础设施
         */
        
        // 创建IO上下文服务
        // 📝 注意：虽然是同步客户端，但boost::asio仍需要io_context
        // 区别是我们不会调用io_context.run()
        boost::asio::io_context ioc;
        
        /*
         * 🎯 第二步：构造服务器连接信息
         */
        
        // 构造服务器endpoint（IP地址 + 端口）
        tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 10086);
        /*
         * 💡 endpoint说明：
         * - IP: 127.0.0.1（本地回环地址，指向本机）
         * - Port: 10086（服务器监听端口，需要与服务器配置一致）
         */
        
        /*
         * 🔌 第三步：创建socket并建立连接
         */
        
        tcp::socket sock(ioc);  // 创建TCP socket
        boost::system::error_code error = boost::asio::error::host_not_found;
        
        // 🔥 关键：同步连接操作（阻塞式）
        sock.connect(remote_ep, error);
        /*
         * 📋 connect操作说明：
         * - 这是同步操作，会阻塞直到连接成功或失败
         * - 与异步的async_connect相对应
         * - 成功：error为空，连接建立
         * - 失败：error包含错误信息
         */
        
        // 🛡️ 连接错误检查
        if (error) {
            cout << "connect failed, code is " << error.value() 
                 << " error msg is " << error.message();
            return 0;
        }
        
        cout << "✅ 成功连接到服务器 127.0.0.1:10086" << endl;
        
        /*
         * 📝 第四步：获取用户输入
         */
        
        std::cout << "Enter message: ";
        char request[MAX_LENGTH];
        std::cin.getline(request, MAX_LENGTH);  // 读取一行用户输入
        size_t request_length = strlen(request);  // 计算消息长度
        
        cout << "📤 准备发送消息: \"" << request << "\" (长度: " << request_length << "字节)" << endl;
        
        /*
         * 📦 第五步：按协议格式封装数据
         * 
         * 协议格式：[2字节长度] + [实际消息内容]
         * 这与服务器的解析逻辑完全对应
         */
        
        char send_data[MAX_LENGTH] = { 0 };  // 发送缓冲区初始化为0
        
        // 🔢 第一部分：写入头部（消息长度）
        memcpy(send_data, &request_length, 2);
        /*
         * 💡 头部处理说明：
         * - 将request_length（size_t类型）的前2字节拷贝到send_data开头
         * - 这样服务器就能知道后续消息体的长度
         * - 与服务器的memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH)对应
         */
        
        // 📄 第二部分：写入消息体（实际内容）
        memcpy(send_data + 2, request, request_length);
        /*
         * 💡 消息体处理说明：
         * - 从send_data的第3个字节开始写入实际消息
         * - 长度为request_length字节
         * - 完整数据格式：[2字节长度][request_length字节内容]
         */
        
        /*
         * 🚀 第六步：发送数据到服务器
         */
        
        // 🔥 关键：同步写入操作（阻塞式）
        boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 2));
        /*
         * 📋 write操作说明：
         * - 这是同步操作，会阻塞直到所有数据发送完成
         * - 与异步的async_write相对应
         * - 发送长度：request_length + 2（消息内容 + 2字节头部）
         * - boost::asio::write保证发送完整数据（不像send可能部分发送）
         */
        
        cout << "📤 数据发送完成！" << endl;
        
        /*
         * 📥 第七步：接收服务器响应
         * 
         * 接收过程分两步（与服务器的发送协议对应）：
         * 1. 先接收头部，获取消息长度
         * 2. 再接收消息体，获取完整内容
         */
        
        // 🔢 步骤7.1：接收响应头部（获取消息长度）
        char reply_head[HEAD_LENGTH];
        size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
        /*
         * 📋 read操作说明：
         * - 这是同步操作，会阻塞直到接收到HEAD_LENGTH（2）字节
         * - 与异步的async_read相对应
         * - boost::asio::read保证接收完整的指定长度数据
         * - reply_length应该等于HEAD_LENGTH（2）
         */
        
        // 🔍 解析头部信息，获取消息体长度
        short msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        /*
         * 💡 头部解析说明：
         * - 从reply_head中提取消息体长度信息
         * - 与服务器的memcpy(&data_len, _recv_head_node->_data, HEAD_LENGTH)对应
         * - msglen就是接下来要接收的消息体字节数
         */
        
        cout << "📥 接收到响应头部，消息体长度: " << msglen << "字节" << endl;
        
        // 📄 步骤7.2：接收消息体（获取完整内容）
        char msg[MAX_LENGTH] = { 0 };
        size_t msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));
        /*
         * 📋 消息体接收说明：
         * - 根据头部信息，准确接收msglen字节的消息体
         * - msg_length应该等于msglen
         * - 现在msg中包含了服务器回显的完整消息
         */
        
        /*
         * 🎉 第八步：显示接收结果
         */
        
        std::cout << "Reply is: ";
        std::cout.write(msg, msglen) << endl;  // 使用write确保输出指定长度
        /*
         * 💡 输出说明：
         * - 使用cout.write而不是cout <<，确保输出exactly msglen字节
         * - 避免因字符串中的\0而提前结束输出
         */
        
        std::cout << "Reply len is " << msglen;
        std::cout << "\n";
        
        cout << "✅ 通信完成，连接即将关闭" << endl;
        
        /*
         * 🔚 第九步：清理资源
         * socket的析构函数会自动关闭连接
         */
        
    }
    catch (std::exception& e) {
        /*
         * ❌ 异常处理：捕获所有可能的网络异常
         */
        std::cerr << "Exception: " << e.what() << endl;
    }
    
    return 0;
}

/*
 * 📚 总结：同步客户端 vs 异步服务器对比
 * 
 * 🔄 协议兼容性：
 * - 客户端发送：[2字节长度] + [消息内容]
 * - 服务器接收：HandleRead解析这个格式
 * - 服务器发送：[2字节长度] + [回显内容] 
 * - 客户端接收：先读头部，再读消息体
 * 
 * 💫 IO模式对比：
 * 
 * | 特性 | 同步客户端 | 异步服务器 |
 * |------|------------|------------|
 * | 操作方式 | 阻塞等待 | 事件驱动 |
 * | 编程复杂度 | 简单直观 | 相对复杂 |
 * | 并发能力 | 单连接 | 多连接 |
 * | 资源使用 | 一个线程一个连接 | 单线程多连接 |
 * | 错误处理 | try-catch | 错误码回调 |
 * | 适用场景 | 简单客户端 | 高并发服务器 |
 * 
 * 🎯 关键理解：
 * 1. 同步操作：调用->等待->完成->返回（阻塞）
 * 2. 异步操作：调用->立即返回->完成时回调（非阻塞）
 * 3. 协议设计：长度前缀解决TCP流式传输的边界问题
 * 4. 错误处理：同步用异常，异步用错误码
 * 
 * 📝 使用方法：
 * 1. 先启动异步服务器（监听10086端口）
 * 2. 再运行这个同步客户端
 * 3. 输入消息，观察回显结果
 * 4. 可以多次运行客户端测试服务器的并发处理能力
 */ 