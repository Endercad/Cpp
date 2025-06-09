/*
 * 多线程同步TCP服务器示例
 * 演示如何使用Boost.Asio创建能处理多个客户端连接的服务器
 * 每个客户端连接都在独立的线程中处理
 */

#include <iostream>     // 标准输入输出流
#include <boost/asio.hpp>   // Boost.Asio网络库
#include <memory>       // 智能指针支持
#include <set>          // STL集合容器
#include <thread>       // C++11线程支持
#include <functional>   // 函数对象支持

using namespace std;
using namespace boost::asio;    // 使用boost::asio命名空间

// 定义消息的最大长度常量
const int MAX_LENGTH = 1024;

// 定义socket智能指针类型，用于自动管理socket的生命周期
typedef std::shared_ptr<ip::tcp::socket> socket_ptr;

// 全局线程集合，存储所有创建的线程，防止线程被意外销毁
std::set<std::shared_ptr<std::thread>> threads;

/*
 * session函数 - 处理单个客户端连接的会话
 * 参数: socket_ptr sock - 与客户端连接的socket智能指针
 * 
 * 这个函数在独立的线程中运行，负责：
 * 1. 接收客户端发送的数据
 * 2. 向客户端发送响应
 * 3. 处理连接断开的情况
 */
void session(socket_ptr sock){
    try{
        // 无限循环处理客户端请求，直到连接断开
        for(;;){
            // Step 1: 准备接收缓冲区
            char data[MAX_LENGTH];
            memset(data, '\0', MAX_LENGTH);  // 初始化缓冲区为空字符
            
            // Step 2: 从客户端读取数据
            boost::system::error_code error;
            /*
             * read_some()函数尝试读取数据，但不保证读取指定数量的字节
             * 它会读取当前可用的数据，返回实际读取的字节数
             * 这与read()函数不同，read()会阻塞直到读取到指定数量的字节
             */
            size_t length = sock->read_some(buffer(data,MAX_LENGTH), error);
            
            // Step 3: 处理读取结果
            if(error == boost::asio::error::eof){
                // EOF表示客户端正常关闭了连接
                cout << "Connection closed by client" << endl;
                break;  // 退出循环，结束此会话
            }
            else if(error){
                // 其他错误情况
                throw boost::system::system_error(error);
            }
            
            // Step 4: 显示接收到的消息
            /*
             * remote_endpoint()获取客户端的端点信息
             * address().to_string()将IP地址转换为字符串
             */
            cout << "Receive message from " << sock->remote_endpoint().address().to_string() 
                 << ": " << data << endl;
            
            // Step 5: 向客户端发送响应
            /*
             * boost::asio::write()是同步写入函数
             * 这里发送固定的响应消息"Message received"
             * 18是消息的字节长度
             */
            boost::asio::write(*sock, buffer("Message received", 18));
        }
    }
    catch(exception& e){
        // 捕获并显示会话中的任何异常
        cout << "Exception in session: " << e.what() << endl;
    }
}

/*
 * server函数 - 服务器主循环
 * 参数: 
 *   io_context& io_context - I/O上下文引用
 *   unsigned short port - 服务器监听的端口号
 * 
 * 这个函数负责：
 * 1. 创建acceptor来监听连接
 * 2. 接受客户端连接
 * 3. 为每个连接创建新的线程处理
 */
void server(io_context& io_context, unsigned short port){
    /*
     * Step 1: 创建TCP acceptor
     * acceptor用于监听指定端口上的连接请求
     * ip::tcp::endpoint(ip::tcp::v4(), port) 创建IPv4端点，监听所有网络接口
     */
    ip::tcp::acceptor acceptor(io_context, ip::tcp::endpoint(ip::tcp::v4(), port));
    
    cout << "服务器启动，监听端口: " << port << endl;
    
    // Step 2: 主服务器循环
    for(;;){
        /*
         * Step 2a: 为新连接创建socket
         * 使用智能指针管理socket，确保自动释放资源
         */
        socket_ptr socket(new ip::tcp::socket(io_context));
        
        /*
         * Step 2b: 等待并接受客户端连接
         * accept()是阻塞函数，会等待直到有客户端连接
         */
        acceptor.accept(*socket);
        cout << "接受新的客户端连接: " << socket->remote_endpoint().address().to_string() << endl;
        
        /*
         * Step 2c: 为新连接创建处理线程
         * 使用std::bind将session函数和socket绑定
         * 将线程指针存储在全局集合中，防止线程对象被销毁
         */
        auto thread_ptr = std::make_shared<std::thread>(std::bind(session, socket));
        threads.insert(thread_ptr);
        
        // 分离线程，让它独立运行
        thread_ptr->detach();
    }
}

/*
 * 主函数 - 程序入口点
 */
int main(){
    try{
        /*
         * Step 1: 创建I/O上下文
         * io_context是Boost.Asio的核心，管理所有I/O操作
         */
        io_context io_context;
        
        /*
         * Step 2: 启动服务器
         * 监听8848端口，等待客户端连接
         */
        server(io_context, 8848);
        
        /*
         * 注意：由于server函数包含无限循环，下面的代码实际不会执行
         * 但保留这个循环是好的编程实践，以防将来需要优雅关闭服务器
         */
        for(auto& thread : threads){
            if(thread->joinable()){
                thread->join();  // 等待所有线程完成
            }
        }
        
        return 0;
    }
    catch(exception& e){
        // 捕获并显示主函数中的任何异常
        cout << "Exception in main: " << e.what() << endl;
    }
    return 0;
}