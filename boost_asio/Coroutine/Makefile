# # Makefile for Boost.Asio Coroutine Example

# CXX = g++
# CXXFLAGS = -std=c++20 -Wall -O2 -fcoroutines
# INCLUDES = 
# LIBS = -lboost_system -lboost_thread -lpthread

# # 源文件
# SOURCES = Coroutine.cpp

# # 目标文件
# TARGET = Coroutine

# # 默认目标
# all: $(TARGET)

# # 编译目标
# $(TARGET): $(SOURCES)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCES) $(LIBS)

# # 清理
# clean:
# 	rm -f $(TARGET)

# # 运行
# run: $(TARGET)
# 	./$(TARGET)

# # 测试编译器是否支持C++20
# test-cpp20:
# 	@echo "测试C++20协程支持..."
# 	@echo 'int main(){return 0;}' | $(CXX) -std=c++20 -fcoroutines -x c++ - -o /tmp/test_cpp20 && echo "✓ C++20协程支持正常" || echo "✗ C++20协程支持有问题"
# 	@rm -f /tmp/test_cpp20

# # 安装依赖（Ubuntu/Debian）
# install-deps:
# 	sudo apt-get update
# 	sudo apt-get install -y libboost-system-dev libboost-dev gcc-10 g++-10

# .PHONY: all clean run test-cpp20 install-deps 