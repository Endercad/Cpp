# Makefile for Boost.Asio Async Server

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
INCLUDES = -I./include
LIBS = -lboost_system -lpthread

# 源文件
SRCDIR = src
SOURCES = $(SRCDIR)/AsyncServer.cpp $(SRCDIR)/Session.cpp

# 目标文件
TARGET = AsyncServer

# 默认目标
all: $(TARGET)

# 编译目标
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCES) $(LIBS)

# 清理
clean:
	rm -f $(TARGET)

# 运行
run: $(TARGET)
	./$(TARGET)

# 安装依赖（Ubuntu/Debian）
install-deps:
	sudo apt-get update
	sudo apt-get install -y libboost-system-dev libboost-dev

.PHONY: all clean run install-deps 