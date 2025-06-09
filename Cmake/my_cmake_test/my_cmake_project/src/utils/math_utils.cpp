/**
 * @file math_utils.cpp
 * @brief 数学工具函数的实现
 * 
 * 该文件实现了math_utils.h中声明的数学函数，
 * 提供基本的整数运算功能。
 */

#include "math_utils.h"  // 包含函数声明
#include <stdexcept>     // 用于std::invalid_argument异常

/**
 * @brief 计算两个整数的和
 * 
 * 简单地返回两个整数参数的算术和
 * 
 * @param a 第一个整数
 * @param b 第二个整数
 * @return int 两数之和 (a + b)
 */
int add(int a, int b) {
    return a + b;
}

/**
 * @brief 计算两个整数的差
 * 
 * 返回第一个整数减去第二个整数的结果
 * 
 * @param a 被减数
 * @param b 减数
 * @return int 两数之差 (a - b)
 */
int subtract(int a, int b) {
    return a - b;
}

/**
 * @brief 计算两个整数的积
 * 
 * 返回两个整数参数的算术乘积
 * 
 * @param a 第一个整数
 * @param b 第二个整数
 * @return int 两数之积 (a * b)
 */
int multiply(int a, int b) {
    return a * b;
}

/**
 * @brief 计算两个整数的商
 * 
 * 返回第一个整数除以第二个整数的结果
 * 如果除数为零，抛出std::invalid_argument异常
 * 
 * @param a 被除数
 * @param b 除数
 * @return double 两数之商 (a / b)，返回double以处理非整除情况
 * @throws std::invalid_argument 当除数b为0时抛出
 */
double divide(int a, int b) {
    // 检查除数是否为零
    if (b == 0) {
        // 除数为零时抛出异常
        throw std::invalid_argument("除数不能为零");
    }
    // 执行除法操作，将结果转换为double以处理非整除情况
    return static_cast<double>(a) / b;
}