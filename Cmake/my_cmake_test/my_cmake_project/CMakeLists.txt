# ==============================================================================
# 主CMakeLists.txt - 项目的顶层CMake配置文件
# ==============================================================================
# 该文件定义了整个项目的构建规则，包括：
# - 项目基本信息设置
# - 编译器和语言标准设置
# - 配置文件生成
# - 包含目录设置
# - 子目录添加
# - 测试启用
# - 安装规则定义
# ==============================================================================

# 指定CMake最低版本要求
# 这确保使用的CMake功能在指定版本中可用
cmake_minimum_required(VERSION 3.10)

# 设置项目名称和版本
# PROJECT_NAME变量将被设置为"MyCMakeProject"
# PROJECT_VERSION将被设置为"1.0.0"
# PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH也会被相应设置
project(MyCMakeProject VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
# 指定使用C++11标准
set(CMAKE_CXX_STANDARD 11)
# 确保编译器不会回退到更早的标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 配置版本信息
# 从模板文件生成实际的头文件，替换模板中的变量
# 源文件：${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
# 目标文件：${CMAKE_CURRENT_BINARY_DIR}/include/config.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/config.h
)

# 添加包含目录
# 这些目录将被添加到编译器的包含路径中
# 源代码目录：确保可以找到项目中的头文件
# 构建目录中的include：确保可以找到生成的config.h
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

# 添加子目录
# 这将处理src目录中的CMakeLists.txt
add_subdirectory(src)

# 启用测试
# 这允许使用CTest运行测试
enable_testing()
# 添加tests子目录，处理其中的CMakeLists.txt
add_subdirectory(tests)

# 安装规则
# 定义如何安装项目的头文件
# 将include/目录下的所有文件安装到目标系统的include/目录
install(DIRECTORY include/ DESTINATION include)

# 打印一些信息
# 这些消息会在CMake配置阶段显示
message(STATUS "项目名称: ${PROJECT_NAME}")
message(STATUS "项目版本: ${PROJECT_VERSION}")
message(STATUS "C++编译器: ${CMAKE_CXX_COMPILER}")