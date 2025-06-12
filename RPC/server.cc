// 
// server.cc - gRPC服务端实现
// Created by leo on 2020/1/31.
//

#include <string>

// 引入gRPC C++库的头文件
#include <grpcpp/grpcpp.h>
// 引入由proto文件生成的gRPC头文件
// 这个文件包含了服务接口的定义
#include "mathtest.grpc.pb.h"

// 使用gRPC命名空间中的核心类
using grpc::Server;           // gRPC服务器类
using grpc::ServerBuilder;    // 服务器构建器，用于配置和创建服务器
using grpc::ServerContext;    // 服务器上下文，包含请求的元数据和状态
using grpc::Status;           // 表示RPC调用的状态（成功/失败）

// 使用我们在proto文件中定义的消息类型
using mathtest::MathTest;     // 生成的服务基类
using mathtest::MathRequest;  // 请求消息类型
using mathtest::MathReply;    // 响应消息类型

// 服务实现类：继承自生成的服务基类
// final关键字表示这个类不能被进一步继承
class MathServiceImplementation final : public MathTest::Service {
    // 重写proto文件中定义的sendRequest方法
    // 这是实际处理客户端请求的业务逻辑
    Status sendRequest(
            ServerContext* context,      // 服务器上下文，包含客户端信息、超时等
            const MathRequest* request,  // 客户端发送的请求消息（只读）
            MathReply* reply            // 要返回给客户端的响应消息（可写）
    ) override {
        // 从请求消息中获取两个操作数
        int a = request->a();
        int b = request->b();

        // 执行业务逻辑：计算两数相乘
        // 将结果设置到响应消息中
        reply->set_result(a * b);

        // 返回成功状态，告诉gRPC框架处理完成
        return Status::OK;
    }
};

// 服务器启动函数
void Run() {
    // 设置服务器监听地址和端口
    // 0.0.0.0 表示监听所有网络接口，5000是端口号
    std::string address("0.0.0.0:5000");
    
    // 创建我们的服务实现实例
    MathServiceImplementation service;

    // 使用ServerBuilder来配置和构建服务器
    ServerBuilder builder;

    // 添加监听端口和安全凭证
    // 这里使用不安全的凭证（仅用于演示，生产环境应使用TLS）
    builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    
    // 注册我们的服务实现到服务器
    builder.RegisterService(&service);

    // 构建并启动服务器
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on port: " << address << std::endl;

    // 阻塞等待，直到服务器被关闭
    // 这里会一直运行，处理客户端的请求
    server->Wait();
}

// 程序入口点
int main(int argc, char** argv) {
    // 启动gRPC服务器
    Run();

    return 0;
}