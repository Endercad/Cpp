# Makefile - gRPC项目编译配置文件
# 用于自动化编译gRPC客户端和服务端程序

# 链接器标志：指定链接时需要的库和路径
LDFLAGS = -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl
# -L/usr/local/lib: 指定库文件搜索路径
# pkg-config --libs: 自动获取protobuf和grpc++的链接库
# -lgrpc++_reflection: 链接gRPC反射库
# -ldl: 链接动态加载库

# 指定使用的C++编译器
CXX = g++

# 预处理器标志：添加头文件搜索路径和宏定义
CPPFLAGS += `pkg-config --cflags protobuf grpc`

# 编译器标志：指定C++标准版本
CXXFLAGS += -std=c++11

# 定义gRPC的C++插件名称
GRPC_CPP_PLUGIN = grpc_cpp_plugin
# 获取gRPC C++插件的完整路径
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

# 默认目标：构建客户端和服务端可执行文件
all: client server

# 构建客户端可执行文件
# 依赖：protobuf生成的对象文件 + gRPC生成的对象文件 + 客户端源码对象文件
client: mathtest.pb.o mathtest.grpc.pb.o client.o
	$(CXX) $^ $(LDFLAGS) -o $@
	# $^: 表示所有依赖文件
	# $@: 表示目标文件名（client）

# 构建服务端可执行文件
# 依赖：protobuf生成的对象文件 + gRPC生成的对象文件 + 服务端源码对象文件
server: mathtest.pb.o mathtest.grpc.pb.o server.o
	$(CXX) $^ $(LDFLAGS) -o $@

# 模式规则：从.proto文件生成gRPC的C++源代码文件
# 这会生成 mathtest.grpc.pb.cc 和 mathtest.grpc.pb.h
%.grpc.pb.cc: %.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<
	# --grpc_out=.: 在当前目录生成gRPC代码
	# --plugin: 指定使用的gRPC插件
	# $<: 表示第一个依赖文件（%.proto）

# 模式规则：从.proto文件生成Protocol Buffers的C++源代码文件
# 这会生成 mathtest.pb.cc 和 mathtest.pb.h
%.pb.cc: %.proto
	protoc --cpp_out=. $<
	# --cpp_out=.: 在当前目录生成C++代码

# 清理目标：删除所有生成的文件
clean:
	rm -f *.o *.pb.cc *.pb.h client server
	# 删除：对象文件(.o) + 生成的源文件(.pb.cc) + 生成的头文件(.pb.h) + 可执行文件 