/**
 * @file 06_scheduler.cpp
 * @brief 协程调度器示例
 * 
 * 这个文件展示了如何使用Scheduler来调度多个协程的执行，包括：
 * 1. 基本的协程调度
 * 2. 优先级调度
 * 3. 多线程并发执行
 * 4. 工作负载均衡
 * 5. 调度器的性能对比
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <atomic>
#include "../include/coroutine/task.h"
#include "../include/coroutine/scheduler.h"

// 全局计数器用于演示
std::atomic<int> global_counter{0};

/**
 * @brief 计算密集型协程任务
 */
ScheduledTask<int> compute_intensive_task(int task_id, int iterations) {
    std::cout << "  [计算任务" << task_id << "] 开始计算，迭代次数: " 
              << iterations << "\n";
    
    int result = 0;
    for (int i = 0; i < iterations; ++i) {
        // 模拟计算密集型工作
        result += i * i;
        
        // 每1000次迭代yield一次，让其他协程有机会执行
        if (i % 1000 == 0 && i > 0) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    
    global_counter.fetch_add(1);
    std::cout << "  [计算任务" << task_id << "] 计算完成，结果: " << result 
              << " (已完成任务数: " << global_counter.load() << ")\n";
    
    co_return result;
}

/**
 * @brief I/O密集型协程任务
 */
ScheduledTask<std::string> io_intensive_task(int task_id, int delay_ms) {
    std::cout << "  [I/O任务" << task_id << "] 开始I/O操作，延迟: " 
              << delay_ms << "ms\n";
    
    // 模拟I/O等待
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    
    std::string result = "I/O任务" + std::to_string(task_id) + "完成";
    global_counter.fetch_add(1);
    
    std::cout << "  [I/O任务" << task_id << "] I/O操作完成: " << result 
              << " (已完成任务数: " << global_counter.load() << ")\n";
    
    co_return result;
}

/**
 * @brief 混合型协程任务
 */
ScheduledTask<void> mixed_task(int task_id) {
    std::cout << "  [混合任务" << task_id << "] 开始执行\n";
    
    // 阶段1：计算
    int compute_result = co_await compute_intensive_task(task_id * 100, 5000);
    
    // 阶段2：I/O
    std::string io_result = co_await io_intensive_task(task_id * 100 + 1, 100);
    
    // 阶段3：更多计算
    int final_result = co_await compute_intensive_task(task_id * 100 + 2, 3000);
    
    std::cout << "  [混合任务" << task_id << "] 所有阶段完成，计算结果: " 
              << compute_result << "，I/O结果: " << io_result 
              << "，最终结果: " << final_result << "\n";
}

/**
 * @brief 演示基本调度器使用
 */
void demonstrate_basic_scheduling() {
    std::cout << "\n=== 1. 基本调度器示例 ===\n";
    
    // 创建调度器（使用2个工作线程）
    Scheduler scheduler(2);
    std::cout << "启动调度器（2个工作线程）...\n";
    scheduler.start();
    
    global_counter.store(0);
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 创建多个计算任务
    std::vector<ScheduledTask<int>> tasks;
    const int task_count = 5;
    
    std::cout << "\n--- 创建 " << task_count << " 个计算任务 ---\n";
    for (int i = 1; i <= task_count; ++i) {
        auto task = compute_intensive_task(i, 10000);
        task.set_scheduler(&scheduler);
        tasks.emplace_back(std::move(task));
    }
    
    // 启动所有任务
    std::cout << "启动所有任务...\n";
    for (auto& task : tasks) {
        task.start(&scheduler);
    }
    
    // 等待所有任务完成
    std::cout << "等待任务完成...\n";
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
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::cout << "  调度器队列大小: " << scheduler.queue_size() 
                      << "，已完成任务: " << global_counter.load() << "/" << task_count << "\n";
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "所有任务完成！总耗时: " << duration.count() << "ms\n";
    
    scheduler.stop();
    std::cout << "调度器已停止\n\n";
}

/**
 * @brief 演示优先级调度
 */
void demonstrate_priority_scheduling() {
    std::cout << "=== 2. 优先级调度示例 ===\n";
    
    Scheduler scheduler(3);
    std::cout << "启动调度器（3个工作线程）...\n";
    scheduler.start();
    
    global_counter.store(0);
    
    std::cout << "\n--- 创建不同优先级的任务 ---\n";
    
    // 创建高优先级任务（优先级10）
    std::vector<ScheduledTask<int>> high_priority_tasks;
    for (int i = 1; i <= 2; ++i) {
        auto task = compute_intensive_task(100 + i, 8000);  // 高优先级任务ID: 101, 102
        high_priority_tasks.emplace_back(std::move(task));
    }
    
    // 创建中优先级任务（优先级5）
    std::vector<ScheduledTask<int>> medium_priority_tasks;
    for (int i = 1; i <= 3; ++i) {
        auto task = compute_intensive_task(200 + i, 8000);  // 中优先级任务ID: 201, 202, 203
        medium_priority_tasks.emplace_back(std::move(task));
    }
    
    // 创建低优先级任务（优先级1）
    std::vector<ScheduledTask<int>> low_priority_tasks;
    for (int i = 1; i <= 3; ++i) {
        auto task = compute_intensive_task(300 + i, 8000);  // 低优先级任务ID: 301, 302, 303
        low_priority_tasks.emplace_back(std::move(task));
    }
    
    // 先启动低优先级任务
    std::cout << "启动低优先级任务...\n";
    for (auto& task : low_priority_tasks) {
        task.start(&scheduler, 1);  // 优先级1
    }
    
    // 短暂延迟后启动中优先级任务
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "启动中优先级任务...\n";
    for (auto& task : medium_priority_tasks) {
        task.start(&scheduler, 5);  // 优先级5
    }
    
    // 再短暂延迟后启动高优先级任务
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "启动高优先级任务...\n";
    for (auto& task : high_priority_tasks) {
        task.start(&scheduler, 10);  // 优先级10
    }
    
    // 等待所有任务完成
    auto wait_for_tasks = [](const std::vector<ScheduledTask<int>>& tasks) {
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
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    };
    
    std::cout << "等待所有任务完成...\n";
    wait_for_tasks(high_priority_tasks);
    wait_for_tasks(medium_priority_tasks);
    wait_for_tasks(low_priority_tasks);
    
    std::cout << "优先级调度示例完成！\n";
    
    scheduler.stop();
    std::cout << "调度器已停止\n\n";
}

/**
 * @brief 演示混合任务调度
 */
void demonstrate_mixed_task_scheduling() {
    std::cout << "=== 3. 混合任务调度示例 ===\n";
    
    Scheduler scheduler(4);
    std::cout << "启动调度器（4个工作线程）...\n";
    scheduler.start();
    
    global_counter.store(0);
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "\n--- 创建混合类型任务 ---\n";
    
    // 创建混合任务
    std::vector<ScheduledTask<void>> mixed_tasks;
    for (int i = 1; i <= 3; ++i) {
        auto task = mixed_task(i);
        mixed_tasks.emplace_back(std::move(task));
    }
    
    // 创建纯I/O任务
    std::vector<ScheduledTask<std::string>> io_tasks;
    for (int i = 1; i <= 4; ++i) {
        auto task = io_intensive_task(500 + i, 200);
        io_tasks.emplace_back(std::move(task));
    }
    
    // 启动所有任务
    std::cout << "启动混合任务...\n";
    for (auto& task : mixed_tasks) {
        task.start(&scheduler, 7);  // 中高优先级
    }
    
    std::cout << "启动I/O任务...\n";
    for (auto& task : io_tasks) {
        task.start(&scheduler, 3);  // 较低优先级
    }
    
    // 监控进度
    std::cout << "监控任务进度...\n";
    while (true) {
        bool all_mixed_done = true;
        for (const auto& task : mixed_tasks) {
            if (!task.done()) {
                all_mixed_done = false;
                break;
            }
        }
        
        bool all_io_done = true;
        for (const auto& task : io_tasks) {
            if (!task.done()) {
                all_io_done = false;
                break;
            }
        }
        
        if (all_mixed_done && all_io_done) {
            break;
        }
        
        std::cout << "  队列大小: " << scheduler.queue_size() 
                  << "，已完成子任务: " << global_counter.load() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "混合任务调度完成！总耗时: " << duration.count() << "ms\n";
    
    scheduler.stop();
    std::cout << "调度器已停止\n\n";
}

/**
 * @brief 演示调度器性能对比
 */
void demonstrate_scheduler_performance() {
    std::cout << "=== 4. 调度器性能对比示例 ===\n";
    
    const int task_count = 10;
    const int iterations = 5000;
    
    // 测试不同线程数的调度器性能
    std::vector<int> thread_counts = {1, 2, 4, 8};
    
    for (int thread_count : thread_counts) {
        std::cout << "\n--- 测试 " << thread_count << " 个工作线程 ---\n";
        
        Scheduler scheduler(thread_count);
        scheduler.start();
        
        global_counter.store(0);
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 创建计算任务
        std::vector<ScheduledTask<int>> tasks;
        for (int i = 1; i <= task_count; ++i) {
            auto task = compute_intensive_task(i, iterations);
            tasks.emplace_back(std::move(task));
        }
        
        // 启动所有任务
        for (auto& task : tasks) {
            task.start(&scheduler);
        }
        
        // 等待完成
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
        
        std::cout << "  " << thread_count << " 线程完成 " << task_count 
                  << " 个任务，耗时: " << duration.count() << "ms\n";
        
        scheduler.stop();
    }
    
    std::cout << "\n调度器性能对比完成\n\n";
}

/**
 * @brief 演示调度器的工作负载均衡
 */
void demonstrate_load_balancing() {
    std::cout << "=== 5. 工作负载均衡示例 ===\n";
    
    Scheduler scheduler(3);
    std::cout << "启动调度器（3个工作线程）...\n";
    scheduler.start();
    
    global_counter.store(0);
    
    std::cout << "\n--- 创建不同工作量的任务 ---\n";
    
    // 创建不同工作量的任务来测试负载均衡
    std::vector<ScheduledTask<int>> tasks;
    std::vector<int> workloads = {2000, 8000, 3000, 12000, 1000, 6000, 9000, 4000};
    
    for (size_t i = 0; i < workloads.size(); ++i) {
        auto task = compute_intensive_task(static_cast<int>(i + 1), workloads[i]);
        tasks.emplace_back(std::move(task));
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // 分批启动任务，观察负载均衡效果
    std::cout << "分批启动任务，观察负载均衡...\n";
    
    for (size_t i = 0; i < tasks.size(); ++i) {
        tasks[i].start(&scheduler);
        std::cout << "  启动任务 " << (i + 1) << "（工作量: " 
                  << workloads[i] << "），队列大小: " << scheduler.queue_size() << "\n";
        
        // 每启动2个任务后暂停一下
        if ((i + 1) % 2 == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // 监控任务完成情况
    std::cout << "\n监控任务完成情况...\n";
    int last_completed = 0;
    while (true) {
        int completed = global_counter.load();
        if (completed > last_completed) {
            std::cout << "  已完成任务: " << completed << "/" << tasks.size() 
                      << "，队列大小: " << scheduler.queue_size() << "\n";
            last_completed = completed;
        }
        
        bool all_done = true;
        for (const auto& task : tasks) {
            if (!task.done()) {
                all_done = false;
                break;
            }
        }
        
        if (all_done) break;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "工作负载均衡测试完成！总耗时: " << duration.count() << "ms\n";
    
    scheduler.stop();
    std::cout << "调度器已停止\n\n";
}

/**
 * @brief 主函数
 */
int main() {
    try {
        std::cout << "🚀 C++20 协程调度器学习\n";
        std::cout << "========================================\n";
        
        std::cout << "\n📚 协程调度器的核心概念：\n";
        std::cout << "1. 多线程调度 - 在多个工作线程上执行协程\n";
        std::cout << "2. 优先级调度 - 根据优先级调度协程执行\n";
        std::cout << "3. 负载均衡 - 在工作线程间平衡工作负载\n";
        std::cout << "4. 任务队列 - 管理待执行的协程任务\n";
        std::cout << "5. 协程上下文切换 - 高效的协程切换机制\n";
        std::cout << "6. 资源管理 - 自动管理线程池和任务生命周期\n";
        
        // 演示各种调度器功能
        demonstrate_basic_scheduling();
        demonstrate_priority_scheduling();
        demonstrate_mixed_task_scheduling();
        demonstrate_scheduler_performance();
        demonstrate_load_balancing();
        
        std::cout << "✅ 协程调度器示例完成！\n";
        std::cout << "\n📖 学习要点总结：\n";
        std::cout << "1. 调度器提供了多线程协程执行环境\n";
        std::cout << "2. 优先级调度确保重要任务优先执行\n";
        std::cout << "3. 适当的线程数可以提高并发性能\n";
        std::cout << "4. 调度器自动处理协程的生命周期管理\n";
        std::cout << "5. 工作负载均衡提高了系统资源利用率\n";
        std::cout << "6. 调度器是构建高性能异步应用的基础\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 