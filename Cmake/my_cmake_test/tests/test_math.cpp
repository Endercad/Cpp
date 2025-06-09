/**
 * @file test_math.cpp
 * @brief 数学工具函数的测试文件
 * 
 * 该文件包含一组测试函数，用于验证math_utils库中
 * 各个数学函数的正确性，包括边界情况和异常处理。
 */

#include <iostream>   // 用于输出测试结果
#include <cassert>    // 用于断言测试条件
#include "../src/utils/math_utils.h"  // 包含被测试的数学工具函数

/**
 * @brief 测试加法函数
 * 
 * 验证add函数在各种输入下的正确性
 */
void test_add() {
    // 测试正数加法
    assert(add(2, 3) == 5);
    // 测试带负数的加法
    assert(add(-1, 1) == 0);
    // 输出测试通过信息
    std::cout << "加法测试通过" << std::endl;
}

/**
 * @brief 测试减法函数
 * 
 * 验证subtract函数在各种输入下的正确性
 */
void test_subtract() {
    // 测试基本减法
    assert(subtract(5, 3) == 2);
    // 测试结果为零的情况
    assert(subtract(1, 1) == 0);
    // 输出测试通过信息
    std::cout << "减法测试通过" << std::endl;
}

/**
 * @brief 测试乘法函数
 * 
 * 验证multiply函数在各种输入下的正确性
 */
void test_multiply() {
    // 测试基本乘法
    assert(multiply(2, 3) == 6);
    // 测试乘以零的情况
    assert(multiply(0, 5) == 0);
    // 输出测试通过信息
    std::cout << "乘法测试通过" << std::endl;
}

/**
 * @brief 测试除法函数
 * 
 * 验证divide函数在各种输入下的正确性，
 * 包括正常除法和除以零的异常处理
 */
void test_divide() {
    // 测试整除情况
    assert(divide(6, 3) == 2.0);
    // 测试非整除情况
    assert(divide(5, 2) == 2.5);
    // 输出测试通过信息
    std::cout << "除法测试通过" << std::endl;
    
    // 测试除以零的异常
    bool caught_exception = false;
    try {
        // 尝试除以零，应该抛出异常
        divide(5, 0);
    } catch (const std::invalid_argument&) {
        // 捕获到预期的异常
        caught_exception = true;
    }
    // 验证是否捕获到异常
    assert(caught_exception);
    // 输出测试通过信息
    std::cout << "除以零异常测试通过" << std::endl;
}

/**
 * @brief 程序入口点
 * 
 * 运行所有测试函数，验证数学工具库的正确性
 * 
 * @return int 程序退出状态码，0表示所有测试通过
 */
int main() {
    // 运行加法测试
    test_add();
    // 运行减法测试
    test_subtract();
    // 运行乘法测试
    test_multiply();
    // 运行除法测试
    test_divide();
    
    // 所有测试通过，输出总结信息
    std::cout << "所有测试通过！" << std::endl;
    return 0;  // 返回成功状态码
}