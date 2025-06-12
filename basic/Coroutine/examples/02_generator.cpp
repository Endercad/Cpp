/**
 * @file 02_generator.cpp
 * @brief 生成器协程示例
 * 
 * 这个文件展示了如何使用Generator协程来创建各种类型的序列生成器，包括：
 * 1. 基本的数字序列生成器
 * 2. 斐波那契数列生成器
 * 3. 素数生成器
 * 4. 文件行生成器
 * 5. 复合生成器（生成器的生成器）
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include "../include/coroutine/generator.h"

/**
 * @brief 基本范围生成器
 * 
 * 生成从start到end的整数序列（不包括end）
 * 类似于Python的range()函数
 * 
 * @param start 起始值
 * @param end   结束值（不包括）
 * @param step  步长
 */
Generator<int> range(int start, int end, int step = 1) {
    std::cout << "  [范围生成器] 开始生成从 " << start << " 到 " << end 
              << " 的序列，步长 " << step << "\n";
    
    for (int i = start; i < end; i += step) {
        std::cout << "  [范围生成器] 产生值: " << i << "\n";
        co_yield i;
    }
    
    std::cout << "  [范围生成器] 序列生成完成\n";
}

/**
 * @brief 斐波那契数列生成器
 * 
 * 生成无限的斐波那契数列
 * 这个例子展示了如何创建无限序列生成器
 * 
 * @param limit 生成数量限制（0表示无限）
 */
Generator<long long> fibonacci(int limit = 0) {
    std::cout << "  [斐波那契生成器] 开始生成斐波那契数列";
    if (limit > 0) {
        std::cout << "，限制 " << limit << " 个数";
    }
    std::cout << "\n";
    
    long long a = 0, b = 1;
    int count = 0;
    
    while (limit == 0 || count < limit) {
        std::cout << "  [斐波那契生成器] 产生第 " << (count + 1) 
                  << " 个斐波那契数: " << a << "\n";
        co_yield a;
        
        long long temp = a + b;
        a = b;
        b = temp;
        count++;
    }
    
    std::cout << "  [斐波那契生成器] 数列生成完成\n";
}

/**
 * @brief 素数生成器
 * 
 * 使用埃拉托色尼筛法生成素数
 * 展示了更复杂的生成器逻辑
 * 
 * @param max_num 最大数值
 */
Generator<int> primes(int max_num) {
    std::cout << "  [素数生成器] 开始生成小于 " << max_num << " 的素数\n";
    
    if (max_num < 2) {
        std::cout << "  [素数生成器] 没有小于 " << max_num << " 的素数\n";
        co_return;
    }
    
    // 使用简单的试除法来生成素数
    std::vector<bool> is_prime(max_num, true);
    is_prime[0] = is_prime[1] = false;
    
    for (int i = 2; i < max_num; ++i) {
        if (is_prime[i]) {
            std::cout << "  [素数生成器] 发现素数: " << i << "\n";
            co_yield i;
            
            // 标记i的倍数为非素数
            for (int j = i * i; j < max_num; j += i) {
                is_prime[j] = false;
            }
        }
    }
    
    std::cout << "  [素数生成器] 素数生成完成\n";
}

/**
 * @brief 字符串单词生成器
 * 
 * 将字符串按空格分割成单词序列
 * 
 * @param text 要分割的文本
 */
Generator<std::string> words(const std::string& text) {
    std::cout << "  [单词生成器] 开始分析文本: \"" << text << "\"\n";
    
    std::string word;
    for (char c : text) {
        if (c == ' ' || c == '\t' || c == '\n') {
            if (!word.empty()) {
                std::cout << "  [单词生成器] 产生单词: \"" << word << "\"\n";
                co_yield word;
                word.clear();
            }
        } else {
            word += c;
        }
    }
    
    // 处理最后一个单词
    if (!word.empty()) {
        std::cout << "  [单词生成器] 产生最后一个单词: \"" << word << "\"\n";
        co_yield word;
    }
    
    std::cout << "  [单词生成器] 文本分析完成\n";
}

/**
 * @brief 变换生成器
 * 
 * 对输入生成器的每个值应用变换函数
 * 这展示了生成器的组合和链式操作
 * 
 * @param gen 输入生成器
 * @param transform 变换函数
 */
template<typename T, typename Func>
Generator<T> map_generator(Generator<T> gen, Func transform) {
    std::cout << "  [变换生成器] 开始应用变换\n";
    
    for (auto value : gen) {
        T transformed = transform(value);
        std::cout << "  [变换生成器] " << value << " -> " << transformed << "\n";
        co_yield transformed;
    }
    
    std::cout << "  [变换生成器] 变换完成\n";
}

/**
 * @brief 过滤生成器
 * 
 * 过滤输入生成器，只产生满足条件的值
 * 
 * @param gen 输入生成器
 * @param predicate 过滤谓词
 */
template<typename T, typename Predicate>
Generator<T> filter_generator(Generator<T> gen, Predicate predicate) {
    std::cout << "  [过滤生成器] 开始过滤\n";
    
    for (auto value : gen) {
        if (predicate(value)) {
            std::cout << "  [过滤生成器] 保留值: " << value << "\n";
            co_yield value;
        } else {
            std::cout << "  [过滤生成器] 过滤掉值: " << value << "\n";
        }
    }
    
    std::cout << "  [过滤生成器] 过滤完成\n";
}

/**
 * @brief 演示基本生成器的使用
 */
void demonstrate_basic_generators() {
    std::cout << "\n=== 1. 基本生成器示例 ===\n";
    
    // 范围生成器示例
    std::cout << "\n--- 范围生成器 ---\n";
    std::cout << "生成 1 到 5 的数字:\n";
    
    for (auto num : range(1, 6)) {
        std::cout << "得到值: " << num << " ";
    }
    std::cout << "\n";
    
    // 步长示例
    std::cout << "\n生成 0 到 10 的偶数:\n";
    for (auto num : range(0, 11, 2)) {
        std::cout << "得到值: " << num << " ";
    }
    std::cout << "\n\n";
}

/**
 * @brief 演示斐波那契生成器
 */
void demonstrate_fibonacci_generator() {
    std::cout << "=== 2. 斐波那契生成器示例 ===\n";
    
    std::cout << "\n生成前10个斐波那契数:\n";
    
    int count = 0;
    for (auto fib : fibonacci(10)) {
        std::cout << "F(" << count++ << ") = " << fib << " ";
        if (count % 5 == 0) std::cout << "\n";
    }
    std::cout << "\n\n";
}

/**
 * @brief 演示素数生成器
 */
void demonstrate_prime_generator() {
    std::cout << "=== 3. 素数生成器示例 ===\n";
    
    std::cout << "\n生成小于30的素数:\n";
    
    std::vector<int> prime_list;
    for (auto prime : primes(30)) {
        prime_list.push_back(prime);
        std::cout << prime << " ";
    }
    std::cout << "\n";
    std::cout << "共找到 " << prime_list.size() << " 个素数\n\n";
}

/**
 * @brief 演示字符串生成器
 */
void demonstrate_string_generator() {
    std::cout << "=== 4. 字符串生成器示例 ===\n";
    
    std::string text = "Hello world this is a coroutine generator example";
    std::cout << "\n分析文本: \"" << text << "\"\n";
    
    std::vector<std::string> word_list;
    for (auto word : words(text)) {
        word_list.push_back(word);
        std::cout << "单词: \"" << word << "\"\n";
    }
    
    std::cout << "共找到 " << word_list.size() << " 个单词\n\n";
}

/**
 * @brief 演示生成器组合
 */
void demonstrate_generator_composition() {
    std::cout << "=== 5. 生成器组合示例 ===\n";
    
    std::cout << "\n--- 变换生成器 ---\n";
    std::cout << "将范围 1-5 的每个数平方:\n";
    
    auto squared_gen = map_generator(range(1, 6), [](int x) { return x * x; });
    
    for (auto value : squared_gen) {
        std::cout << "平方值: " << value << " ";
    }
    std::cout << "\n";
    
    std::cout << "\n--- 过滤生成器 ---\n";
    std::cout << "从 1-20 中过滤出偶数:\n";
    
    auto even_gen = filter_generator(range(1, 21), [](int x) { return x % 2 == 0; });
    
    for (auto value : even_gen) {
        std::cout << "偶数: " << value << " ";
    }
    std::cout << "\n";
    
    std::cout << "\n--- 链式组合 ---\n";
    std::cout << "生成 1-10，过滤奇数，然后平方:\n";
    
    auto chain_gen = map_generator(
        filter_generator(range(1, 11), [](int x) { return x % 2 == 1; }),
        [](int x) { return x * x; }
    );
    
    for (auto value : chain_gen) {
        std::cout << "结果: " << value << " ";
    }
    std::cout << "\n\n";
}

/**
 * @brief 演示生成器的延迟计算特性
 */
void demonstrate_lazy_evaluation() {
    std::cout << "=== 6. 延迟计算特性演示 ===\n";
    
    std::cout << "\n创建一个大范围的生成器（但不会立即计算）:\n";
    
    auto big_range = range(1, 1000000);  // 创建生成器，但还没开始计算
    std::cout << "生成器已创建，但还没有开始生成数值\n";
    
    std::cout << "现在只取前5个值:\n";
    int count = 0;
    for (auto value : big_range) {
        std::cout << "值: " << value << " ";
        if (++count >= 5) break;  // 只取前5个
    }
    std::cout << "\n";
    std::cout << "可以看到，只计算了需要的值，这就是延迟计算的优势\n\n";
}

/**
 * @brief 演示生成器的内存效率
 */
void demonstrate_memory_efficiency() {
    std::cout << "=== 7. 内存效率演示 ===\n";
    
    std::cout << "\n传统方法 vs 生成器方法的对比:\n";
    
    // 传统方法：一次性生成所有斐波那契数
    std::cout << "传统方法 - 一次性生成前20个斐波那契数到vector:\n";
    std::vector<long long> fib_vector;
    long long a = 0, b = 1;
    for (int i = 0; i < 20; ++i) {
        fib_vector.push_back(a);
        long long temp = a + b;
        a = b;
        b = temp;
    }
    std::cout << "vector大小: " << fib_vector.size() << " 个元素，占用内存较多\n";
    
    // 生成器方法：按需生成
    std::cout << "\n生成器方法 - 按需生成斐波那契数:\n";
    int count = 0;
    for (auto fib : fibonacci(20)) {
        std::cout << fib << " ";
        if (++count % 10 == 0) std::cout << "\n";
    }
    std::cout << "\n生成器只保存当前状态，内存使用量恒定\n\n";
}

/**
 * @brief 主函数
 */
int main() {
    try {
        std::cout << "🚀 C++20 生成器协程学习\n";
        std::cout << "========================================\n";
        
        std::cout << "\n📚 生成器协程的核心概念：\n";
        std::cout << "1. 延迟计算 - 只在需要时计算下一个值\n";
        std::cout << "2. 内存效率 - 不需要存储整个序列\n";
        std::cout << "3. 可组合性 - 可以组合多个生成器\n";
        std::cout << "4. 无限序列 - 可以表示无限长的序列\n";
        std::cout << "5. 状态保持 - 在yield点保持执行状态\n";
        
        // 演示各种生成器示例
        demonstrate_basic_generators();
        demonstrate_fibonacci_generator();
        demonstrate_prime_generator();
        demonstrate_string_generator();
        demonstrate_generator_composition();
        demonstrate_lazy_evaluation();
        demonstrate_memory_efficiency();
        
        std::cout << "✅ 生成器协程示例完成！\n";
        std::cout << "\n📖 学习要点总结：\n";
        std::cout << "1. Generator通过co_yield产生值序列\n";
        std::cout << "2. 支持范围for循环，使用简单直观\n";
        std::cout << "3. 延迟计算提供了内存和性能优势\n";
        std::cout << "4. 可以轻松组合和链式操作\n";
        std::cout << "5. 适用于处理大数据集或无限序列\n";
        std::cout << "6. 协程状态自动管理，简化了复杂迭代逻辑\n\n";
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 