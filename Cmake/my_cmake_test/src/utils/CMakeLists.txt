# ==============================================================================
# src/utils/CMakeLists.txt - 工具库目录的CMake配置文件
# ==============================================================================
# 该文件定义了数学工具库的构建规则，包括：
# - 创建库目标
# - 设置库属性
# - 定义安装规则
# ==============================================================================

# 创建数学工具库
# 从math_utils.cpp和math_utils.h创建名为math_utils的库
# 默认创建的是静态库(.a)，除非设置了BUILD_SHARED_LIBS变量
add_library(math_utils
    math_utils.cpp
    math_utils.h
)

# 设置库的属性
# VERSION: 设置库的版本号，与项目版本相同
# SOVERSION: 设置库的ABI版本号，使用主版本号
# 这些属性主要用于共享库(.so)，影响库文件的命名
set_target_properties(math_utils PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# 安装库和头文件
# 定义如何安装math_utils库
# LIBRARY DESTINATION lib: 安装共享库(.so)到lib目录
# ARCHIVE DESTINATION lib: 安装静态库(.a)到lib目录
install(TARGETS math_utils
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装头文件到include/utils目录
install(FILES math_utils.h DESTINATION include/utils)