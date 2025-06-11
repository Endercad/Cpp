#include <boost/asio/co_spawn.hpp>  // 这个头文件是用来创建协程的
#include <boost/asio/detached.hpp> //这个头文件是用来分离协程的
#include <boost/asio/io_context.hpp> //这个头文件是用来创建io_context的
#include <boost/asio/ip/tcp.hpp> //这个头文件是用来创建tcp的
#include <boost/asio/signal_set.hpp> //这个头文件是用来创建信号的
#include <boost/asio/write.hpp> //这个头文件是用来写入数据的
#include <boost/asio/awaitable.hpp> // awaitable协程类型

#include <iostream>


using boost::asio::ip::tcp; //using 和 using namespace 的区别：using 是用来引入命名空间中的成员，而 using namespace 是用来引入整个命名空间。
using boost::asio::awaitable; //允许异步等待协程的完成
using boost::asio::co_spawn; 
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;  //namespace关键字，用于定义命名空间，这里定义了this_coro命名空间，用于访问当前协程的执行器

awaitable<void> echo_server(tcp::socket socket){
    try{
        char data[1024];
        for(;;){
            size_t length = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
            // 通过协程的方式，传入use_awaitable，等待异步操作完成，并返回结果，如果不用use_awaitable，async_read_some需要传入一个回调函数，当有数据读取时，会调用回调函数，创建tcp::socket对象，用于与客户端通信
            co_await boost::asio::async_write(socket, boost::asio::buffer(data, length), use_awaitable);
        }
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
awaitable<void> listener(){
    try {
        auto executor = co_await this_coro::executor; //获取当前协程的调度器
        tcp::acceptor acceptor(executor, {tcp::v4(), 8848}); //创建tcp::acceptor对象，用于监听8848端口
        for(;;){
            tcp::socket socket = co_await acceptor.async_accept(use_awaitable); //等待客户端连接，当有客户端连接时，会调用回调函数，创建tcp::socket对象，用于与客户端通信
            //co_await 关键字将异步操作转换为同步操作，等待异步操作完成，并返回结果，如果不用co_await，async_accept需要传入一个回调函数，当有客户端连接时，会调用回调函数，创建tcp::socket对象，用于与客户端通信
            //co_await会等待异步操作完成，此时会阻塞当前协程，释放当前协程的执行权，让出CPU时间片，让其他协程有机会执行，当异步操作完成时，会恢复当前协程的执行权，继续执行下面的代码
            //use_awaitable 是completion token，用于等待异步操作完成，并返回结果，如果不用use_awaitable，async_accept需要传入一个回调函数，当有客户端连接时，会调用回调函数，创建tcp::socket对象，用于与客户端通信
            co_spawn(executor, echo_server(std::move(socket)), detached); //创建协程，并分离协程，当协程完成时，不会等待协程的完成，而是继续执行下面的代码,detached 表示协程是独立的，不会等待协程的完成，而是继续执行下面的代码
            //如果不用协程，那么不能保证发送的有序性，因为异步操作是并发执行的，不能保证发送的有序性，而协程是串行执行的，可以保证发送的有序性
        }
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

int main(){
    try{
        boost::asio::io_context io_context; 
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM); //创建信号集，用于捕获SIGINT和SIGTERM信号，当收到这些信号时，会调用回调函数，停止io_context的运行
        signals.async_wait([&](auto, auto){ io_context.stop(); }); //设置信号集的回调函数，当收到SIGINT和SIGTERM信号时，会调用回调函数，停止io_context的运行,回调参数使用auto自动推导，当信号触发时，回调接收两个参数，第一个参数是错误码，第二个参数是信号值，通过 io_context.stop() 终止异步事件循环，这是处理信号后停止程序的标准做法
        co_spawn(io_context, listener(), detached); //创建协程，并分离协程，当协程完成时，不会等待协程的完成，而是继续执行下面的代码,detached 表示协程是独立的，不会等待协程的完成，而是继续执行下面的代码
        io_context.run(); //运行io_context，开始处理异步事件
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}