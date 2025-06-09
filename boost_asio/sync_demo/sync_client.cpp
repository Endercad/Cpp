/*
 * 同步TCP客户端示例
 * 演示如何使用Boost.Asio建立TCP连接、发送数据和接收响应
 */

#include <iostream>     // 标准输入输出流
#include <boost/asio.hpp>   // Boost.Asio网络库，提供异步I/O功能

using namespace std;
using namespace boost::asio;    // 使用boost::asio命名空间

// 定义消息的最大长度常量
const int MAX_LENGTH = 1024;

int main(){
    try{
        /*
         * Step 1: 创建I/O上下文对象
         * io_context是Boost.Asio的核心类，负责管理所有异步操作
         * 即使在同步编程中也需要它来创建socket等对象
         */
        io_context io_context;
        
        /*
         * Step 2: 创建服务器端点(endpoint)
         * endpoint包含了服务器的IP地址和端口号
         * ip::make_address("127.0.0.1") - 创建本地回环地址
         * 8848 - 服务器监听的端口号
         */
        ip::tcp::endpoint endpoint(ip::make_address("127.0.0.1"), 8848);
        
        /*
         * Step 3: 创建TCP socket对象
         * socket是网络通信的端点，用于发送和接收数据
         * 需要传入io_context来管理socket的生命周期
         */
        ip::tcp::socket socket(io_context);
        
        /*  
         * Step 4: 连接到服务器
         * error_code用于捕获可能出现的连接错误
         * socket.connect()方法尝试连接到指定的endpoint
         */
        boost::system::error_code error = error::host_not_found;
        socket.connect(endpoint, error);
        
        // 检查连接是否成功
        if(error){
            throw boost::system::system_error(error);
        }
        
        /*
         * Step 5: 获取用户输入
         * 从标准输入读取用户要发送的消息
         */
        cout << "Enter message: ";
        char request[MAX_LENGTH];
        cin.getline(request, MAX_LENGTH);   // 读取一整行输入
        size_t request_length = strlen(request);    // 计算消息长度
        
        /*
         * Step 6: 发送数据到服务器
         * boost::asio::write()是同步写入函数，会阻塞直到所有数据发送完成
         * buffer()函数创建一个缓冲区包装器，包装要发送的数据
         */
        boost::asio::write(socket, buffer(request, request_length));
        
        /*
         * Step 7: 接收服务器的响应
         * 创建接收缓冲区并使用同步读取函数
         */
        char reply[MAX_LENGTH];
        size_t reply_length = boost::asio::read(socket, buffer(reply, request_length), error);
        
        /*
         * Step 8: 处理接收结果
         * 检查是否是正常的连接关闭或其他错误
         */
        if(error == boost::asio::error::eof){
            // EOF表示连接被对方正常关闭
            cout << "Connection closed by server" << endl;
        }
        else if(error){
            // 其他错误情况
            throw boost::system::system_error(error);
        }
        
        /*
         * Step 9: 显示服务器响应
         * 使用cout.write()确保输出指定长度的数据
         */
        cout << "Reply is: ";
        cout.write(reply, reply_length);
        cout << endl;
        
        return 0;
    }
    catch(exception& e){
        // 捕获并显示所有异常
        cout << "Exception: " << e.what() << endl;
    }
    return 0;
}