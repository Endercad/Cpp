/**
 * @file main.cpp
 * @brief 项目主程序文件
 * 
 * 该文件包含main函数，是程序的入口点。
 * 它演示了如何使用math_utils库中的数学函数，
 * 并展示了如何访问config.h中定义的项目信息。
 */

#include <iostream>  // 用于输入输出流操作
#include "utils/math_utils.h"  // 包含数学工具函数
#include "config.h"  // 包含项目配置信息

/**
 * @brief 程序入口点
 * 
 * 显示项目信息并演示数学工具库的各种功能
 * 
 * @return int 程序退出状态码，0表示成功
 */
int main() {
    // 显示项目信息，使用config.h中定义的宏
    std::cout << "欢迎使用 " << PROJECT_NAME << " 版本 " << PROJECT_VERSION << std::endl;
    
    // 定义测试数据
    int a = 10, b = 5;
    
    // 演示加法函数
    std::cout << "加法: " << a << " + " << b << " = " << add(a, b) << std::endl;
    
    // 演示减法函数
    std::cout << "减法: " << a << " - " << b << " = " << subtract(a, b) << std::endl;
    
    // 演示乘法函数
    std::cout << "乘法: " << a << " * " << b << " = " << multiply(a, b) << std::endl;
    
    // 演示除法函数
    std::cout << "除法: " << a << " / " << b << " = " << divide(a, b) << std::endl;
    
    return 0;  // 返回成功状态码
}