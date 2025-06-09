# ==============================================================================
# tests/CMakeLists.txt - 测试目录的CMake配置文件
# ==============================================================================
# 该文件定义了测试的构建和运行规则，包括：
# - 创建测试可执行文件
# - 链接被测试的库
# - 添加测试到CTest框架
# ==============================================================================

# 添加测试可执行文件
# 从test_math.cpp源文件创建名为test_math的可执行文件
add_executable(test_math test_math.cpp)

# 链接数学工具库
# 将math_utils库链接到test_math可执行文件
# PRIVATE关键字表示math_utils库仅被test_math直接使用
target_link_libraries(test_math PRIVATE math_utils)

# 添加测试
# 向CTest框架注册一个名为MathTest的测试
# 该测试将运行test_math可执行文件
# 如果test_math返回零退出码，测试被视为通过
add_test(NAME MathTest COMMAND test_math)