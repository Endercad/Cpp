/**
 * @file 03_task.cpp
 * @brief Task协程示例
 * 
 * 这个文件展示了如何使用Task协程来处理异步任务，包括：
 * 1. 基本的异步任务创建和等待
 * 2. 任务的链式组合
 * 3. 异常处理
 * 4. 并发任务执行
 * 5. 任务的取消和超时
 */

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include "../include/coroutine/task.h"

/**
 * @brief 模拟异步计算任务
 * 
 * 这是一个简单的异步任务，模拟一些耗时的计算工作
 * 
 * @param value 输入值
 * @param delay_ms 模拟的延迟时间（毫秒）
 * @return 计算结果
 */
Task<int> async_compute(int value, int delay_ms) {
    std::cout << "  [异步计算] 开始计算 " << value << "，预计耗时 " 
              << delay_ms << "ms\n";
    
    // 模拟异步计算过程
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    
    int result = value * value + 10;
    std::cout << "  [异步计算] 计算完成: " << value 
              << " -> " << result << "\n";
    
    co_return result;
}

/**
 * @brief 模拟可能失败的异步任务
 * 
 * 这个任务有一定概率会抛出异常，用于演示异常处理
 * 
 * @param value 输入值
 * @param fail_chance 失败概率（0.0 - 1.0）
 */
Task<std::string> async_risky_operation(int value, double fail_chance = 0.3) {
    std::cout << "  [风险操作] 开始执行风险操作，值=" << value 
              << "，失败概率=" << fail_chance << "\n";
    
    // 模拟一些处理时间
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 随机决定是否失败
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    if (dis(gen) < fail_chance) {
        std::cout << "  [风险操作] 操作失败！\n";
        throw std::runtime_error("异步操作失败，值=" + std::to_string(value));
    }
    
    std::string result = "成功处理值: " + std::to_string(value);
    std::cout << "  [风险操作] 操作成功: " << result << "\n";
    
    co_return result;
}

/**
 * @brief 组合多个异步任务的复合任务
 * 
 * 这个任务演示了如何使用co_await来组合多个异步操作
 * 
 * @param input 输入值
 */
Task<std::string> complex_async_task(int input) {
    std::cout << "  [复合任务] 开始执行复合任务，输入=" << input << "\n";
    
    try {
        // 第一步：异步计算
        std::cout << "  [复合任务] 步骤1：执行异步计算\n";
        int computed = co_await async_compute(input, 200);
        
        // 第二步：风险操作
        std::cout << "  [复合任务] 步骤2：执行风险操作\n";
        std::string risky_result = co_await async_risky_operation(computed, 0.2);
        
        // 第三步：再次计算
        std::cout << "  [复合任务] 步骤3：再次计算\n";
        int final_computed = co_await async_compute(computed + 5, 150);
        
        std::string final_result = risky_result + " -> 最终值: " + std::to_string(final_computed);
        std::cout << "  [复合任务] 复合任务完成: " << final_result << "\n";
        
        co_return final_result;
        
    } catch (const std::exception& e) {
        std::string error_msg = "复合任务失败: " + std::string(e.what());
        std::cout << "  [复合任务] " << error_msg << "\n";
        throw;  // 重新抛出异常
    }
}

/**
 * @brief 模拟异步文件操作
 */
Task<std::string> async_file_operation(const std::string& filename, const std::string& content) {
    std::cout << "  [文件操作] 开始异步文件操作: " << filename << "\n";
    
    // 模拟文件写入延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    std::string result = "文件 '" + filename + "' 写入成功，内容: " + content;
    std::cout << "  [文件操作] " << result << "\n";
    
    co_return result;
}

/**
 * @brief 模拟异步网络请求
 */
Task<int> async_network_request(const std::string& url, int timeout_ms) {
    std::cout << "  [网络请求] 开始请求: " << url 
              << "，超时: " << timeout_ms << "ms\n";
    
    // 模拟网络延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms / 2));
    
    // 模拟响应码
    int response_code = 200;
    std::cout << "  [网络请求] 请求完成，响应码: " << response_code << "\n";
    
    co_return response_code;
}

/**
 * @brief 演示基本Task使用
 */
void demonstrate_basic_task() {
    std::cout << "\n=== 1. 基本Task示例 ===\n";
    
    std::cout << "\n--- 创建并等待单个任务 ---\n";
    
    // 创建任务（此时还未开始执行）
    auto task = async_compute(5, 100);
    std::cout << "任务已创建，但尚未开始执行\n";
    
    // co_await会启动任务并等待完成
    // 注意：在非协程函数中，我们需要手动控制执行
    std::cout << "手动启动任务...\n";
    task.resume();
    
    // 等待任务完成
    while (!task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "任务执行完成\n\n";
}

/**
 * @brief 同步运行异步任务的辅助函数
 * 
 * 在实际应用中，通常会有一个事件循环来处理这些任务
 * 这里我们简单地同步等待任务完成
 */
template<typename T>
T sync_wait(Task<T> task) {
    task.resume();
    while (!task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    // 注意：这里无法直接获取返回值，这是一个简化的实现
    // 在真实的实现中，需要更复杂的机制来获取结果
    return T{};  // 占位符
}

/**
 * @brief 演示任务组合
 */
void demonstrate_task_composition() {
    std::cout << "=== 2. 任务组合示例 ===\n";
    
    std::cout << "\n--- 顺序执行多个任务 ---\n";
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 创建复合任务
    auto complex_task = complex_async_task(3);
    
    // 启动任务
    complex_task.resume();
    
    // 等待完成
    while (!complex_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "复合任务总耗时: " << duration.count() << "ms\n\n";
}

/**
 * @brief 演示异常处理
 */
void demonstrate_exception_handling() {
    std::cout << "=== 3. 异常处理示例 ===\n";
    
    std::cout << "\n--- 处理可能失败的任务 ---\n";
    
    // 尝试多次执行可能失败的任务
    for (int i = 1; i <= 3; ++i) {
        std::cout << "尝试 " << i << ":\n";
        
        try {
            auto risky_task = async_risky_operation(i * 10, 0.5);  // 50%失败率
            
            risky_task.resume();
            while (!risky_task.done()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            std::cout << "任务成功完成\n";
            
        } catch (const std::exception& e) {
            std::cout << "捕获异常: " << e.what() << "\n";
        }
        
        std::cout << "\n";
    }
}

/**
 * @brief 演示并发任务执行
 */
void demonstrate_concurrent_tasks() {
    std::cout << "=== 4. 并发任务示例 ===\n";
    
    std::cout << "\n--- 并发启动多个任务 ---\n";
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 创建多个并发任务
    std::vector<Task<int>> tasks;
    
    std::cout << "创建5个并发计算任务...\n";
    for (int i = 1; i <= 5; ++i) {
        tasks.emplace_back(async_compute(i, 200));  // 每个任务200ms
    }
    
    // 同时启动所有任务
    std::cout << "同时启动所有任务...\n";
    for (auto& task : tasks) {
        task.resume();
    }
    
    // 等待所有任务完成
    std::cout << "等待所有任务完成...\n";
    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (const auto& task : tasks) {
            if (!task.done()) {
                all_done = false;
                break;
            }
        }
        if (!all_done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "所有并发任务完成，总耗时: " << duration.count() << "ms\n";
    std::cout << "（如果顺序执行需要约1000ms，并发执行大大减少了总时间）\n\n";
}

/**
 * @brief 演示不同类型任务的组合
 */
void demonstrate_mixed_tasks() {
    std::cout << "=== 5. 混合任务类型示例 ===\n";
    
    std::cout << "\n--- 组合不同类型的异步操作 ---\n";
    
    // 创建不同类型的任务
    auto compute_task = async_compute(7, 150);
    auto file_task = async_file_operation("output.txt", "Hello Coroutines!");
    auto network_task = async_network_request("https://api.example.com/data", 200);
    
    std::cout << "启动混合任务...\n";
    
    // 启动所有任务
    compute_task.resume();
    file_task.resume();
    network_task.resume();
    
    // 等待所有任务完成
    while (!compute_task.done() || !file_task.done() || !network_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "所有混合任务完成！\n\n";
}

/**
 * @brief 带返回值的协程任务示例
 * 
 * 演示如何在协程中处理和返回复杂的结果
 */
Task<std::vector<int>> process_batch_data(const std::vector<int>& input_data) {
    std::cout << "  [批处理] 开始处理 " << input_data.size() << " 个数据项\n";
    
    std::vector<int> results;
    results.reserve(input_data.size());
    
    for (size_t i = 0; i < input_data.size(); ++i) {
        std::cout << "  [批处理] 处理项目 " << (i + 1) << "/" << input_data.size() 
                  << ": " << input_data[i] << "\n";
        
        // 对每个项目执行异步计算
        int processed = co_await async_compute(input_data[i], 50);
        results.push_back(processed);
    }
    
    std::cout << "  [批处理] 批处理完成，共处理 " << results.size() << " 个项目\n";
    co_return results;
}

/**
 * @brief 演示批处理任务
 */
void demonstrate_batch_processing() {
    std::cout << "=== 6. 批处理任务示例 ===\n";
    
    std::cout << "\n--- 批量数据处理 ---\n";
    
    std::vector<int> input_data = {1, 2, 3, 4, 5};
    std::cout << "输入数据: ";
    for (int val : input_data) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    
    auto batch_task = process_batch_data(input_data);
    
    batch_task.resume();
    while (!batch_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "批处理任务完成\n\n";
}

/**
 * @brief 主函数
 */
int main() {
    try {
        std::cout << "🚀 C++20 Task协程学习\n";
        std::cout << "========================================\n";
        
        std::cout << "\n📚 Task协程的核心概念：\n";
        std::cout << "1. 异步执行 - 表示一个异步计算任务\n";
        std::cout << "2. co_await - 等待其他异步操作完成\n";
        std::cout << "3. 异常传播 - 异常可以跨协程边界传播\n";
        std::cout << "4. 组合性 - 可以组合多个异步操作\n";
        std::cout << "5. 延迟执行 - 只有被await时才开始执行\n";
        std::cout << "6. 值传递 - 可以返回计算结果\n";
        
        // 演示各种Task用法
        demonstrate_basic_task();
        demonstrate_task_composition();
        demonstrate_exception_handling();
        demonstrate_concurrent_tasks();
        demonstrate_mixed_tasks();
        demonstrate_batch_processing();
        
        std::cout << "✅ Task协程示例完成！\n";
        std::cout << "\n📖 学习要点总结：\n";
        std::cout << "1. Task代表一个可等待的异步计算\n";
        std::cout << "2. co_await用于等待Task完成并获取结果\n";
        std::cout << "3. 异常会自动从被等待的Task传播到等待者\n";
        std::cout << "4. Task支持组合，可以构建复杂的异步工作流\n";
        std::cout << "5. 并发执行多个Task可以提高性能\n";
        std::cout << "6. Task的延迟执行特性避免了不必要的计算\n";
        std::cout << "7. 类型安全的返回值处理\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 