/**
 * @file 05_channels.cpp
 * @brief 协程通道示例
 * 
 * 这个文件展示了如何使用Channel进行协程间通信，包括：
 * 1. 基本的生产者-消费者模式
 * 2. 多生产者-多消费者场景
 * 3. 通道的缓冲和流控
 * 4. 通道的关闭和错误处理
 * 5. 复杂的数据流处理管道
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "../include/coroutine/task.h"
#include "../include/coroutine/channels.h"

/**
 * @brief 简单的生产者协程
 */
Task<void> simple_producer(std::shared_ptr<Channel<int>> channel, int start, int count) {
    std::cout << "  [生产者] 开始生产数据，起始值: " << start 
              << "，数量: " << count << "\n";
    
    for (int i = 0; i < count; ++i) {
        int value = start + i;
        std::cout << "  [生产者] 发送: " << value << "\n";
        
        try {
            co_await channel->send(value);
            std::cout << "  [生产者] 发送成功: " << value << "\n";
        } catch (const std::exception& e) {
            std::cout << "  [生产者] 发送失败: " << e.what() << "\n";
            break;
        }
        
        // 模拟生产延迟
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "  [生产者] 生产完成\n";
}

/**
 * @brief 简单的消费者协程
 */
Task<void> simple_consumer(std::shared_ptr<Channel<int>> channel, const std::string& name) {
    std::cout << "  [消费者" << name << "] 开始消费数据\n";
    
    int count = 0;
    while (true) {
        try {
            int value = co_await channel->receive();
            count++;
            std::cout << "  [消费者" << name << "] 接收到: " << value 
                      << " (第" << count << "个)\n";
            
            // 模拟处理延迟
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
            
        } catch (const std::exception& e) {
            std::cout << "  [消费者" << name << "] 接收失败: " << e.what() << "\n";
            break;
        }
    }
    
    std::cout << "  [消费者" << name << "] 消费完成，共处理 " << count << " 个数据\n";
}

/**
 * @brief 演示基本的生产者-消费者模式
 */
Task<void> demonstrate_basic_producer_consumer() {
    std::cout << "\n=== 1. 基本生产者-消费者示例 ===\n";
    
    // 创建一个容量为3的通道
    auto channel = make_channel<int>(3);
    
    std::cout << "\n--- 启动生产者和消费者 ---\n";
    
    // 创建生产者任务
    auto producer_task = simple_producer(channel, 1, 5);
    
    // 创建消费者任务
    auto consumer_task = simple_consumer(channel, "A");
    
    // 启动任务
    producer_task.resume();
    consumer_task.resume();
    
    // 等待生产者完成
    while (!producer_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "\n生产者完成，关闭通道...\n";
    channel->close();
    
    // 等待消费者完成
    while (!consumer_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "基本生产者-消费者示例完成\n\n";
}

/**
 * @brief 批量数据生产者
 */
Task<void> batch_producer(std::shared_ptr<Channel<std::vector<int>>> channel, 
                          int batch_count, int batch_size) {
    std::cout << "  [批量生产者] 开始生产 " << batch_count 
              << " 个批次，每批 " << batch_size << " 个数据\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    for (int batch = 0; batch < batch_count; ++batch) {
        std::vector<int> data;
        data.reserve(batch_size);
        
        for (int i = 0; i < batch_size; ++i) {
            data.push_back(dis(gen));
        }
        
        std::cout << "  [批量生产者] 发送第 " << (batch + 1) << " 批数据，大小: " 
                  << data.size() << "\n";
        
        try {
            co_await channel->send(std::move(data));
            std::cout << "  [批量生产者] 第 " << (batch + 1) << " 批发送成功\n";
        } catch (const std::exception& e) {
            std::cout << "  [批量生产者] 发送失败: " << e.what() << "\n";
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    std::cout << "  [批量生产者] 生产完成\n";
}

/**
 * @brief 批量数据消费者
 */
Task<void> batch_consumer(std::shared_ptr<Channel<std::vector<int>>> channel, 
                          const std::string& name) {
    std::cout << "  [批量消费者" << name << "] 开始消费批量数据\n";
    
    int total_processed = 0;
    int batch_count = 0;
    
    while (true) {
        try {
            auto batch = co_await channel->receive();
            batch_count++;
            
            std::cout << "  [批量消费者" << name << "] 接收到第 " << batch_count 
                      << " 批数据，大小: " << batch.size() << "\n";
            
            // 处理批量数据
            for (int value : batch) {
                total_processed++;
                // 模拟处理每个数据项
            }
            
            std::cout << "  [批量消费者" << name << "] 第 " << batch_count 
                      << " 批处理完成\n";
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            std::cout << "  [批量消费者" << name << "] 接收失败: " << e.what() << "\n";
            break;
        }
    }
    
    std::cout << "  [批量消费者" << name << "] 消费完成，共处理 " 
              << total_processed << " 个数据项，" << batch_count << " 个批次\n";
}

/**
 * @brief 演示批量数据处理
 */
Task<void> demonstrate_batch_processing() {
    std::cout << "=== 2. 批量数据处理示例 ===\n";
    
    // 创建批量数据通道
    auto channel = make_channel<std::vector<int>>(2);
    
    std::cout << "\n--- 启动批量生产者和消费者 ---\n";
    
    // 创建任务
    auto producer_task = batch_producer(channel, 4, 10);
    auto consumer_task = batch_consumer(channel, "Batch");
    
    // 启动任务
    producer_task.resume();
    consumer_task.resume();
    
    // 等待生产者完成
    while (!producer_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "\n批量生产者完成，关闭通道...\n";
    channel->close();
    
    // 等待消费者完成
    while (!consumer_task.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "批量数据处理示例完成\n\n";
}

/**
 * @brief 多生产者协程
 */
Task<void> multi_producer(std::shared_ptr<Channel<std::string>> channel, 
                          const std::string& producer_id, int message_count) {
    std::cout << "  [生产者" << producer_id << "] 开始生产 " 
              << message_count << " 条消息\n";
    
    for (int i = 1; i <= message_count; ++i) {
        std::string message = "来自生产者" + producer_id + "的消息" + std::to_string(i);
        
        try {
            co_await channel->send(message);
            std::cout << "  [生产者" << producer_id << "] 发送: " << message << "\n";
        } catch (const std::exception& e) {
            std::cout << "  [生产者" << producer_id << "] 发送失败: " << e.what() << "\n";
            break;
        }
        
        // 随机延迟
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(50, 200);
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
    }
    
    std::cout << "  [生产者" << producer_id << "] 生产完成\n";
}

/**
 * @brief 多消费者协程
 */
Task<void> multi_consumer(std::shared_ptr<Channel<std::string>> channel, 
                          const std::string& consumer_id) {
    std::cout << "  [消费者" << consumer_id << "] 开始消费\n";
    
    int count = 0;
    while (true) {
        try {
            std::string message = co_await channel->receive();
            count++;
            std::cout << "  [消费者" << consumer_id << "] 处理: " << message 
                      << " (第" << count << "个)\n";
            
            // 模拟处理时间
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            std::cout << "  [消费者" << consumer_id << "] 接收失败: " << e.what() << "\n";
            break;
        }
    }
    
    std::cout << "  [消费者" << consumer_id << "] 消费完成，共处理 " << count << " 条消息\n";
}

/**
 * @brief 演示多生产者-多消费者模式
 */
Task<void> demonstrate_multi_producer_consumer() {
    std::cout << "=== 3. 多生产者-多消费者示例 ===\n";
    
    // 创建消息通道
    auto channel = make_channel<std::string>(5);
    
    std::cout << "\n--- 启动多个生产者和消费者 ---\n";
    
    // 创建多个生产者
    std::vector<Task<void>> producers;
    for (int i = 1; i <= 3; ++i) {
        producers.emplace_back(multi_producer(channel, std::to_string(i), 3));
    }
    
    // 创建多个消费者
    std::vector<Task<void>> consumers;
    for (int i = 1; i <= 2; ++i) {
        consumers.emplace_back(multi_consumer(channel, std::to_string(i)));
    }
    
    // 启动所有任务
    for (auto& producer : producers) {
        producer.resume();
    }
    for (auto& consumer : consumers) {
        consumer.resume();
    }
    
    // 等待所有生产者完成
    bool all_producers_done = false;
    while (!all_producers_done) {
        all_producers_done = true;
        for (const auto& producer : producers) {
            if (!producer.done()) {
                all_producers_done = false;
                break;
            }
        }
        if (!all_producers_done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    std::cout << "\n所有生产者完成，关闭通道...\n";
    channel->close();
    
    // 等待所有消费者完成
    bool all_consumers_done = false;
    while (!all_consumers_done) {
        all_consumers_done = true;
        for (const auto& consumer : consumers) {
            if (!consumer.done()) {
                all_consumers_done = false;
                break;
            }
        }
        if (!all_consumers_done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    std::cout << "多生产者-多消费者示例完成\n\n";
}

/**
 * @brief 数据处理管道阶段
 */
Task<void> pipeline_stage(std::shared_ptr<Channel<int>> input_channel,
                          std::shared_ptr<Channel<int>> output_channel,
                          const std::string& stage_name,
                          std::function<int(int)> transform) {
    std::cout << "  [管道阶段" << stage_name << "] 开始处理\n";
    
    int count = 0;
    while (true) {
        try {
            int input = co_await input_channel->receive();
            count++;
            
            // 应用变换
            int output = transform(input);
            
            std::cout << "  [管道阶段" << stage_name << "] 处理: " 
                      << input << " -> " << output << " (第" << count << "个)\n";
            
            // 发送到下一阶段
            co_await output_channel->send(output);
            
        } catch (const std::exception& e) {
            std::cout << "  [管道阶段" << stage_name << "] 处理结束: " << e.what() << "\n";
            break;
        }
    }
    
    std::cout << "  [管道阶段" << stage_name << "] 完成处理，共处理 " << count << " 个数据\n";
    
    // 关闭输出通道
    output_channel->close();
}

/**
 * @brief 演示数据处理管道
 */
Task<void> demonstrate_processing_pipeline() {
    std::cout << "=== 4. 数据处理管道示例 ===\n";
    
    std::cout << "\n--- 创建三阶段数据处理管道 ---\n";
    std::cout << "管道流程: 数据源 -> 乘法器 -> 加法器 -> 输出\n";
    
    // 创建管道各阶段之间的通道
    auto source_to_multiply = make_channel<int>(3);
    auto multiply_to_add = make_channel<int>(3);
    auto add_to_output = make_channel<int>(3);
    
    // 创建数据源
    auto data_source = [](std::shared_ptr<Channel<int>> channel) -> Task<void> {
        std::cout << "  [数据源] 开始生成数据\n";
        
        for (int i = 1; i <= 10; ++i) {
            std::cout << "  [数据源] 生成: " << i << "\n";
            co_await channel->send(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "  [数据源] 数据生成完成\n";
        channel->close();
    }(source_to_multiply);
    
    // 创建乘法器阶段 (x * 2)
    auto multiplier = pipeline_stage(
        source_to_multiply, 
        multiply_to_add,
        "乘法器",
        [](int x) { return x * 2; }
    );
    
    // 创建加法器阶段 (x + 10)
    auto adder = pipeline_stage(
        multiply_to_add,
        add_to_output,
        "加法器", 
        [](int x) { return x + 10; }
    );
    
    // 创建输出消费者
    auto output_consumer = [](std::shared_ptr<Channel<int>> channel) -> Task<void> {
        std::cout << "  [输出消费者] 开始接收最终结果\n";
        
        int count = 0;
        while (true) {
            try {
                int result = co_await channel->receive();
                count++;
                std::cout << "  [输出消费者] 最终结果: " << result 
                          << " (第" << count << "个)\n";
            } catch (const std::exception& e) {
                std::cout << "  [输出消费者] 接收结束: " << e.what() << "\n";
                break;
            }
        }
        
        std::cout << "  [输出消费者] 完成，共接收 " << count << " 个结果\n";
    }(add_to_output);
    
    // 启动所有阶段
    data_source.resume();
    multiplier.resume();
    adder.resume();
    output_consumer.resume();
    
    // 等待所有阶段完成
    while (!data_source.done() || !multiplier.done() || 
           !adder.done() || !output_consumer.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "数据处理管道示例完成\n\n";
}

/**
 * @brief 演示通道的缓冲效果
 */
Task<void> demonstrate_channel_buffering() {
    std::cout << "=== 5. 通道缓冲效果示例 ===\n";
    
    std::cout << "\n--- 对比不同缓冲大小的效果 ---\n";
    
    // 测试无缓冲通道
    auto test_buffering = [](size_t buffer_size) -> Task<void> {
        std::cout << "\n测试缓冲大小: " << buffer_size << "\n";
        
        auto channel = make_channel<int>(buffer_size);
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 快速生产者
        auto fast_producer = [](std::shared_ptr<Channel<int>> ch) -> Task<void> {
            for (int i = 1; i <= 5; ++i) {
                std::cout << "    [快速生产者] 尝试发送: " << i << "\n";
                co_await ch->send(i);
                std::cout << "    [快速生产者] 发送成功: " << i << "\n";
            }
            ch->close();
        }(channel);
        
        // 慢速消费者
        auto slow_consumer = [](std::shared_ptr<Channel<int>> ch) -> Task<void> {
            while (true) {
                try {
                    int value = co_await ch->receive();
                    std::cout << "    [慢速消费者] 接收到: " << value << "\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 慢速处理
                } catch (const std::exception& e) {
                    std::cout << "    [慢速消费者] 接收结束\n";
                    break;
                }
            }
        }(channel);
        
        // 启动任务
        fast_producer.resume();
        slow_consumer.resume();
        
        // 等待完成
        while (!fast_producer.done() || !slow_consumer.done()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "    缓冲大小 " << buffer_size << " 总耗时: " << duration.count() << "ms\n";
    };
    
    // 测试不同的缓冲大小
    co_await test_buffering(0);  // 无缓冲
    co_await test_buffering(2);  // 小缓冲
    co_await test_buffering(10); // 大缓冲
    
    std::cout << "通道缓冲效果示例完成\n\n";
}

/**
 * @brief 主函数 - 运行通道示例
 */
Task<void> run_channel_examples() {
    std::cout << "🚀 C++20 协程通道学习\n";
    std::cout << "========================================\n";
    
    std::cout << "\n📚 协程通道的核心概念：\n";
    std::cout << "1. 同步通信 - 协程间的安全数据传输\n";
    std::cout << "2. 流控制 - 通过缓冲管理数据流速\n";
    std::cout << "3. 阻塞语义 - 发送和接收的阻塞行为\n";
    std::cout << "4. 多对多 - 支持多生产者和多消费者\n";
    std::cout << "5. 管道模式 - 构建数据处理管道\n";
    std::cout << "6. 资源管理 - 自动的通道生命周期管理\n";
    
    // 演示各种通道用法
    co_await demonstrate_basic_producer_consumer();
    co_await demonstrate_batch_processing();
    co_await demonstrate_multi_producer_consumer();
    co_await demonstrate_processing_pipeline();
    co_await demonstrate_channel_buffering();
    
    std::cout << "✅ 协程通道示例完成！\n";
    std::cout << "\n📖 学习要点总结：\n";
    std::cout << "1. Channel提供了协程间安全的通信机制\n";
    std::cout << "2. 缓冲大小影响通道的性能和内存使用\n";
    std::cout << "3. 通道关闭是协调协程结束的重要机制\n";
    std::cout << "4. 多生产者-多消费者模式适用于负载均衡\n";
    std::cout << "5. 管道模式便于构建复杂的数据处理流程\n";
    std::cout << "6. 适当的错误处理确保通道通信的可靠性\n\n";
}

/**
 * @brief 主函数
 */
int main() {
    try {
        auto main_task = run_channel_examples();
        
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