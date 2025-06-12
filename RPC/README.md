# gRPC入门项目

这是一个简单的gRPC入门示例，演示了如何使用gRPC进行客户端-服务端通信。

## 项目结构

```
.
├── mathtest.proto      # Protocol Buffers定义文件
├── server.cc          # gRPC服务端实现
├── client.cc          # gRPC客户端实现
├── Makefile           # 编译配置文件
└── README.md          # 项目说明文件
```

## gRPC核心概念

### 1. Protocol Buffers (.proto文件)
- **作用**：定义服务接口和数据结构
- **语法**：使用proto3语法
- **生成**：通过protoc编译器生成C++代码

### 2. 服务定义
```protobuf
service MathTest {
    rpc sendRequest (MathRequest) returns (MathReply) {}
}
```
- **service**：定义gRPC服务
- **rpc**：定义远程过程调用方法
- **一元RPC**：一个请求对应一个响应

### 3. 消息定义
```protobuf
message MathRequest {
    int32 a = 1;  // 字段标识符，不是默认值
    int32 b = 2;
}
```

### 4. 服务端实现
- **继承生成的Service基类**
- **重写RPC方法**：实现具体的业务逻辑
- **ServerBuilder**：构建和配置服务器
- **阻塞运行**：server->Wait()等待客户端请求

### 5. 客户端实现
- **创建Channel**：连接到服务器
- **创建Stub**：服务代理对象
- **调用远程方法**：如同调用本地方法
- **处理响应**：检查状态并获取结果

## 编译和运行

### 1. 编译项目
```bash
make
```
这个命令会：
- 从.proto文件生成C++源代码
- 编译生成的代码和用户代码
- 链接必要的gRPC和protobuf库
- 生成client和server可执行文件

### 2. 运行服务端
```bash
./server
```
输出：`Server listening on port: 0.0.0.0:5000`

### 3. 运行客户端
在另一个终端运行：
```bash
./client
```
输出：`Answer received: 5 * 10 = 50`

### 4. 清理生成的文件
```bash
make clean
```

## 工作流程

1. **protoc编译器**读取.proto文件
2. **生成C++代码**：包含消息类和服务基类
3. **用户实现**：继承服务基类，实现业务逻辑
4. **编译链接**：生成最终的可执行文件
5. **运行时**：客户端通过网络调用服务端方法

## 生成的文件说明

编译后会生成以下文件：
- `mathtest.pb.h` / `mathtest.pb.cc`：Protocol Buffers消息类
- `mathtest.grpc.pb.h` / `mathtest.grpc.pb.cc`：gRPC服务类
- `*.o`：编译生成的对象文件
- `client` / `server`：最终的可执行文件

## 关键优势

1. **语言无关**：支持多种编程语言
2. **高性能**：基于HTTP/2协议
3. **类型安全**：强类型的消息定义
4. **代码生成**：自动生成客户端和服务端代码
5. **易于使用**：远程调用如同本地方法调用 