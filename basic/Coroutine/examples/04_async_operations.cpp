/**
 * @file 04_async_operations.cpp
 * @brief 异步操作协程示例
 * 
 * 这个文件展示了各种异步操作的协程化包装，包括：
 * 1. 异步延迟操作
 * 2. 异步文件I/O
 * 3. 异步网络操作模拟
 * 4. 异步操作的组合
 * 5. 超时和取消机制
 */

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include "../include/coroutine/task.h"
#include "../include/coroutine/async_ops.h"

/**
 * @brief 演示基本异步延迟操作
 */
Task<void> demonstrate_delay_operations() {
    std::cout << "\n=== 1. 异步延迟操作示例 ===\n";
    
    std::cout << "\n--- 基本延迟操作 ---\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    std::cout << "开始1秒延迟...\n";
    co_await delay(std::chrono::milliseconds(1000));
    std::cout << "1秒延迟完成！\n";
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "实际耗时: " << duration.count() << "ms\n";
    
    std::cout << "\n--- 多个连续延迟 ---\n";
    for (int i = 1; i <= 3; ++i) {
        std::cout << "延迟 " << i << " (200ms)...\n";
        co_await delay(std::chrono::milliseconds(200));
        std::cout << "延迟 " << i << " 完成\n";
    }
    
    std::cout << "所有延迟操作完成\n\n";
}

/**
 * @brief 演示异步文件操作
 */
Task<void> demonstrate_file_operations() {
    std::cout << "=== 2. 异步文件操作示例 ===\n";
    
    std::cout << "\n--- 异步文件写入 ---\n";
    std::string content = "这是一个协程写入的文件内容\n";
    content += "包含多行文本\n";
    content += "展示异步文件操作的能力\n";
    
    try {
        std::cout << "开始异步写入文件...\n";
        co_await write_file("test_output.txt", content);
        std::cout << "文件写入完成！\n";
        
        std::cout << "\n--- 异步文件读取 ---\n";
        std::cout << "开始异步读取文件...\n";
        std::string read_content = co_await read_file("test_output.txt");
        
        std::cout << "文件读取完成！内容:\n";
        std::cout << "--- 文件内容开始 ---\n";
        std::cout << read_content;
        std::cout << "--- 文件内容结束 ---\n";
        
    } catch (const std::exception& e) {
        std::cout << "文件操作失败: " << e.what() << "\n";
    }
    
    std::cout << "\n";
}

/**
 * @brief 模拟复杂的异步网络操作
 */
Task<std::string> simulate_http_request(const std::string& url, int timeout_ms) {
    std::cout << "  [HTTP请求] 开始请求: " << url << "\n";
    
    // 模拟网络延迟
    co_await delay(std::chrono::milliseconds(timeout_ms));
    
    // 模拟随机响应
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(200, 500);
    
    int status_code = dis(gen);
    std::string response = "HTTP " + std::to_string(status_code) + " - 来自 " + url + " 的响应";
    
    std::cout << "  [HTTP请求] 请求完成: " << response << "\n";
    co_return response;
}

/**
 * @brief 演示并发异步操作
 */
Task<void> demonstrate_concurrent_operations() {
    std::cout << "=== 3. 并发异步操作示例 ===\n";
    
    std::cout << "\n--- 并发HTTP请求 ---\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    // 创建多个并发的HTTP请求任务
    std::vector<Task<std::string>> tasks;
    std::vector<std::string> urls = {
        "https://api.example1.com/data",
        "https://api.example2.com/users", 
        "https://api.example3.com/posts",
        "https://api.example4.com/comments"
    };
    
    std::cout << "发起 " << urls.size() << " 个并发请求...\n";
    
    // 创建所有任务
    for (const auto& url : urls) {
        tasks.emplace_back(simulate_http_request(url, 300));
    }
    
    // 等待所有任务完成
    std::vector<std::string> responses;
    for (auto& task : tasks) {
        std::string response = co_await task;
        responses.push_back(response);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\n所有请求完成，总耗时: " << duration.count() << "ms\n";
    std::cout << "收到 " << responses.size() << " 个响应:\n";
    for (size_t i = 0; i < responses.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << responses[i] << "\n";
    }
    
    std::cout << "\n";
}

/**
 * @brief 模拟数据库异步操作
 */
Task<std::vector<std::string>> async_database_query(const std::string& query, int delay_ms) {
    std::cout << "  [数据库] 执行查询: " << query << "\n";
    
    // 模拟数据库查询延迟
    co_await delay(std::chrono::milliseconds(delay_ms));
    
    // 模拟查询结果
    std::vector<std::string> results;
    if (query.find("users") != std::string::npos) {
        results = {"user1", "user2", "user3"};
    } else if (query.find("posts") != std::string::npos) {
        results = {"post1", "post2", "post3", "post4"};
    } else {
        results = {"result1", "result2"};
    }
    
    std::cout << "  [数据库] 查询完成，返回 " << results.size() << " 条记录\n";
    co_return results;
}

/**
 * @brief 演示复杂异步工作流
 */
Task<void> demonstrate_complex_workflow() {
    std::cout << "=== 4. 复杂异步工作流示例 ===\n";
    
    std::cout << "\n--- 用户数据处理流程 ---\n";
    
    try {
        // 步骤1：获取用户列表
        std::cout << "步骤1：获取用户列表\n";
        auto users = co_await async_database_query("SELECT * FROM users", 200);
        
        // 步骤2：为每个用户获取详细信息
        std::cout << "步骤2：获取用户详细信息\n";
        std::vector<Task<std::vector<std::string>>> detail_tasks;
        
        for (const auto& user : users) {
            std::string detail_query = "SELECT details FROM user_details WHERE user='" + user + "'";
            detail_tasks.emplace_back(async_database_query(detail_query, 150));
        }
        
        // 等待所有详细信息查询完成
        std::vector<std::vector<std::string>> all_details;
        for (auto& task : detail_tasks) {
            auto details = co_await task;
            all_details.push_back(details);
        }
        
        // 步骤3：生成报告文件
        std::cout << "步骤3：生成报告文件\n";
        std::string report = "用户报告\n";
        report += "=========\n";
        
        for (size_t i = 0; i < users.size(); ++i) {
            report += "用户: " + users[i] + "\n";
            report += "详细信息: ";
            for (const auto& detail : all_details[i]) {
                report += detail + " ";
            }
            report += "\n\n";
        }
        
        co_await write_file("user_report.txt", report);
        
        std::cout << "工作流完成！报告已保存到 user_report.txt\n";
        
    } catch (const std::exception& e) {
        std::cout << "工作流执行失败: " << e.what() << "\n";
    }
    
    std::cout << "\n";
}

/**
 * @brief 演示异步操作的超时处理
 */
Task<void> demonstrate_timeout_handling() {
    std::cout << "=== 5. 超时处理示例 ===\n";
    
    std::cout << "\n--- 模拟超时场景 ---\n";
    
    // 模拟一个可能超时的操作
    auto timeout_task = [](int delay_ms, const std::string& operation) -> Task<std::string> {
        std::cout << "  [超时测试] 开始操作: " << operation 
                  << "，预计耗时: " << delay_ms << "ms\n";
        
        co_await delay(std::chrono::milliseconds(delay_ms));
        
        std::string result = operation + " 完成";
        std::cout << "  [超时测试] " << result << "\n";
        co_return result;
    };
    
    // 测试不同的超时场景
    std::vector<std::pair<int, std::string>> test_cases = {
        {100, "快速操作"},
        {500, "中等操作"}, 
        {1500, "慢速操作"}
    };
    
    for (const auto& [delay, operation] : test_cases) {
        std::cout << "\n测试: " << operation << "\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            // 这里简化了超时实现，实际应用中需要更复杂的超时机制
            auto task = timeout_task(delay, operation);
            std::string result = co_await task;
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            std::cout << "操作成功完成，耗时: " << duration.count() << "ms\n";
            
        } catch (const std::exception& e) {
            std::cout << "操作失败: " << e.what() << "\n";
        }
    }
    
    std::cout << "\n";
}

/**
 * @brief 演示批量异步操作处理
 */
Task<void> demonstrate_batch_operations() {
    std::cout << "=== 6. 批量异步操作示例 ===\n";
    
    std::cout << "\n--- 批量文件处理 ---\n";
    
    // 创建多个文件写入任务
    std::vector<std::pair<std::string, std::string>> files = {
        {"file1.txt", "内容1：这是第一个文件"},
        {"file2.txt", "内容2：这是第二个文件"},  
        {"file3.txt", "内容3：这是第三个文件"},
        {"file4.txt", "内容4：这是第四个文件"}
    };
    
    std::cout << "开始批量写入 " << files.size() << " 个文件...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // 并发执行所有文件写入
    std::vector<Task<void>> write_tasks;
    for (const auto& [filename, content] : files) {
        write_tasks.emplace_back([](std::string fname, std::string data) -> Task<void> {
            co_await write_file(fname, data);
            std::cout << "  文件写入完成: " << fname << "\n";
        }(filename, content));
    }
    
    // 等待所有写入完成
    for (auto& task : write_tasks) {
        co_await task;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "批量文件写入完成，总耗时: " << duration.count() << "ms\n";
    
    std::cout << "\n--- 批量文件读取 ---\n";
    
    // 并发读取所有文件
    std::vector<Task<std::string>> read_tasks;
    for (const auto& [filename, _] : files) {
        read_tasks.emplace_back(read_file(filename));
    }
    
    std::cout << "开始批量读取文件...\n";
    std::vector<std::string> contents;
    for (auto& task : read_tasks) {
        try {
            std::string content = co_await task;
            contents.push_back(content);
        } catch (const std::exception& e) {
            std::cout << "读取文件失败: " << e.what() << "\n";
            contents.push_back("读取失败");
        }
    }
    
    std::cout << "批量读取完成，共读取 " << contents.size() << " 个文件\n";
    
    std::cout << "\n";
}

/**
 * @brief 主函数 - 运行异步操作协程
 */
Task<void> run_async_examples() {
    std::cout << "🚀 C++20 异步操作协程学习\n";
    std::cout << "========================================\n";
    
    std::cout << "\n📚 异步操作协程的核心概念：\n";
    std::cout << "1. Awaitable对象 - 可以被co_await的异步操作\n";
    std::cout << "2. 异步I/O - 非阻塞的文件和网络操作\n";
    std::cout << "3. 并发执行 - 同时执行多个异步操作\n";
    std::cout << "4. 异常处理 - 异步操作中的错误处理\n";
    std::cout << "5. 超时控制 - 限制异步操作的执行时间\n";
    std::cout << "6. 工作流编排 - 组合多个异步操作\n";
    
    // 演示各种异步操作
    co_await demonstrate_delay_operations();
    co_await demonstrate_file_operations();
    co_await demonstrate_concurrent_operations();
    co_await demonstrate_complex_workflow();
    co_await demonstrate_timeout_handling();
    co_await demonstrate_batch_operations();
    
    std::cout << "✅ 异步操作协程示例完成！\n";
    std::cout << "\n📖 学习要点总结：\n";
    std::cout << "1. 异步操作通过awaitable接口与协程集成\n";
    std::cout << "2. co_await使异步代码看起来像同步代码\n";
    std::cout << "3. 并发执行多个异步操作可以显著提高性能\n";
    std::cout << "4. 异常处理在异步环境中仍然有效\n";
    std::cout << "5. 复杂的异步工作流可以通过协程简化\n";
    std::cout << "6. 适当的超时和错误处理是健壮异步程序的关键\n\n";
}

/**
 * @brief 主函数
 */
int main() {
    try {
        auto main_task = run_async_examples();
        
        // 手动运行主协程
        main_task.resume();
        while (!main_task.done()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 