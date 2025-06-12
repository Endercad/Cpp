//
// client.cc - gRPC客户端实现
// Created by leo on 2020/1/31.
//

#include <string>

// 引入gRPC C++客户端库
#include <grpcpp/grpcpp.h>
// 引入由proto文件生成的gRPC头文件
#include "mathtest.grpc.pb.h"

// 使用gRPC客户端相关的核心类
using grpc::Channel;          // 表示到服务器的连接通道
using grpc::ClientContext;    // 客户端上下文，用于设置请求的元数据、超时等
using grpc::Status;           // RPC调用的状态结果

// 使用proto文件生成的类型
using mathtest::MathTest;     // 生成的服务客户端桩（Stub）
using mathtest::MathRequest;  // 请求消息类型
using mathtest::MathReply;    // 响应消息类型

// 客户端类：封装与gRPC服务器的通信逻辑
class MathTestClient {
public:
    // 构造函数：接收一个到服务器的连接通道
    // 并创建一个服务桩（Stub），用于调用远程方法
    MathTestClient(std::shared_ptr<Channel> channel) : stub_(MathTest::NewStub(channel)) {}

    // 发送请求到服务器的方法
    // 参数：两个要相乘的整数
    // 返回：服务器计算的结果，失败时返回-1
    int sendRequest(int a, int b) {
        // 创建请求消息对象
        MathRequest request;

        // 设置请求消息的字段值
        request.set_a(a);
        request.set_b(b);

        // 创建响应消息对象，用于接收服务器的回复
        MathReply reply;

        // 创建客户端上下文，可以设置超时、元数据等
        ClientContext context;

        // 调用远程方法：通过桩发送请求到服务器
        // 这是一个同步调用，会阻塞直到服务器响应
        Status status = stub_->sendRequest(&context, request, &reply);

        // 检查调用是否成功
        if(status.ok()){
            // 成功：返回服务器计算的结果
            return reply.result();
        } else {
            // 失败：打印错误信息并返回-1
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
        }
    }

private:
    // 服务桩：这是客户端与服务器通信的代理对象
    // 它封装了网络通信的细节，提供类似本地方法调用的接口
    std::unique_ptr<MathTest::Stub> stub_;
};

// 客户端运行函数
void Run() {
    // 设置要连接的服务器地址和端口
    std::string address("0.0.0.0:5000");
    
    // 创建客户端实例
    // grpc::CreateChannel() 创建到服务器的连接通道
    // grpc::InsecureChannelCredentials() 使用不安全的连接（仅用于演示）
    MathTestClient client(
            grpc::CreateChannel(
                    address,
                    grpc::InsecureChannelCredentials()
            )
    );

    int response;

    // 准备要发送给服务器进行计算的两个数
    int a = 5;
    int b = 10;

    // 调用远程方法，发送请求到服务器
    // 这里看起来像调用本地方法，但实际上是网络调用
    response = client.sendRequest(a, b);
    
    // 打印从服务器接收到的计算结果
    std::cout << "Answer received: " << a << " * " << b << " = " << response << std::endl;
}

// 程序入口点
int main(int argc, char* argv[]){
    // 启动客户端
    Run();

    return 0;
}