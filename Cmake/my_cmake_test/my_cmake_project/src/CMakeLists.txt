# ==============================================================================
# src/CMakeLists.txt - 源代码目录的CMake配置文件
# ==============================================================================
# 该文件定义了源代码的构建规则，包括：
# - 添加子目录
# - 创建可执行文件
# - 链接依赖库
# - 定义安装规则
# ==============================================================================

# 添加工具库子目录
# 这将处理utils目录中的CMakeLists.txt，构建math_utils库
add_subdirectory(utils)

# 创建可执行文件
# 从main.cpp源文件创建名为my_app的可执行文件
add_executable(my_app main.cpp)

# 链接工具库
# 将math_utils库链接到my_app可执行文件
# PRIVATE关键字表示math_utils库仅被my_app直接使用，不会暴露给依赖my_app的目标
target_link_libraries(my_app PRIVATE math_utils)

# 安装可执行文件
# 定义如何安装my_app可执行文件
# DESTINATION bin表示安装到目标系统的bin目录
install(TARGETS my_app DESTINATION bin)