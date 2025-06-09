/**
 * @file math_utils.h
 * @brief 数学工具函数的头文件
 * 
 * 该文件声明了一组基本的数学运算函数，
 * 包括加法、减法、乘法和除法操作。
 */

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

/**
 * @brief 计算两个整数的和
 * 
 * @param a 第一个整数
 * @param b 第二个整数
 * @return int 两数之和
 */
int add(int a, int b);

/**
 * @brief 计算两个整数的差
 * 
 * @param a 被减数
 * @param b 减数
 * @return int 两数之差 (a - b)
 */
int subtract(int a, int b);

/**
 * @brief 计算两个整数的积
 * 
 * @param a 第一个整数
 * @param b 第二个整数
 * @return int 两数之积
 */
int multiply(int a, int b);

/**
 * @brief 计算两个整数的商
 * 
 * 如果除数为零，将抛出std::invalid_argument异常
 * 
 * @param a 被除数
 * @param b 除数
 * @return double 两数之商 (a / b)，返回double以处理非整除情况
 * @throws std::invalid_argument 当除数b为0时抛出
 */
double divide(int a, int b);

#endif // MATH_UTILS_H