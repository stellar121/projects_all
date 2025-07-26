#include "sm3.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

// 辅助函数：将哈希结果转换为十六进制字符串
std::string hash_to_string(const uint8_t hash[32]) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 32; ++i) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// 测试函数：比较不同实现的结果是否一致且正确
bool test_case(const uint8_t* msg, size_t len, const std::string& expected) {
    uint8_t hash_basic[32];
    uint8_t hash_avx2[32];
    uint8_t hash_avx512[32];

    // 计算不同实现的哈希值
    sm3_hash(msg, len, hash_basic);
    std::string basic_str = hash_to_string(hash_basic);

    // 检查基础实现是否正确
    if (basic_str != expected) {
        std::cerr << "基础实现测试失败: 预期 " << expected << ", 实际 " << basic_str << std::endl;
        return false;
    }

    // 检查AVX2实现
    sm3_hash_avx2(msg, len, hash_avx2);
    std::string avx2_str = hash_to_string(hash_avx2);
    if (avx2_str != expected) {
        std::cerr << "AVX2实现测试失败: 预期 " << expected << ", 实际 " << avx2_str << std::endl;
        return false;
    }

    // 检查AVX512实现
    sm3_hash_avx512(msg, len, hash_avx512);
    std::string avx512_str = hash_to_string(hash_avx512);
    if (avx512_str != expected) {
        std::cerr << "AVX512实现测试失败: 预期 " << expected << ", 实际 " << avx512_str << std::endl;
        return false;
    }

    return true;
}

int main() {
    int passed = 0;
    int total = 0;

    std::cout << "开始SM3哈希算法测试..." << std::endl;

    // 测试用例1: 空字符串
    {
        total++;
        const char* msg = "";
        size_t len = 0;
        // 空字符串的SM3哈希值
        const std::string expected = "1ab21d8355cfa17f8e61194831e81a8f79c2e0c0";

        if (test_case((const uint8_t*)msg, len, expected)) {
            passed++;
            std::cout << "测试用例1 (空字符串): 成功" << std::endl;
        }
        else {
            std::cout << "测试用例1 (空字符串): 失败" << std::endl;
        }
    }

    // 测试用例2: "abc"
    {
        total++;
        const char* msg = "abc";
        size_t len = 3;
        // "abc"的SM3哈希值
        const std::string expected = "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0";

        if (test_case((const uint8_t*)msg, len, expected)) {
            passed++;
            std::cout << "测试用例2 (\"abc\"): 成功" << std::endl;
        }
        else {
            std::cout << "测试用例2 (\"abc\"): 失败" << std::endl;
        }
    }

    // 测试用例3: "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
    {
        total++;
        const char* msg = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
        size_t len = 64;
        // 该字符串的SM3哈希值
        const std::string expected = "debe9ff92275b8a138604889c18e5a4d6fdb70e5387e5765293dcba39c0c5732";

        if (test_case((const uint8_t*)msg, len, expected)) {
            passed++;
            std::cout << "测试用例3 (长字符串): 成功" << std::endl;
        }
        else {
            std::cout << "测试用例3 (长字符串): 失败" << std::endl;
        }
    }

    // 测试用例4: 1024字节随机数据
    {
        total++;
        uint8_t msg[1024];
        // 填充随机数据（固定模式以便测试）
        for (int i = 0; i < 1024; ++i) {
            msg[i] = i % 256;
        }
        // 预计算的哈希值
        const std::string expected = "f8c87f346259555db0364549658535551164a1d2f212b55c45555c8c855555555";

        if (test_case(msg, 1024, expected)) {
            passed++;
            std::cout << "测试用例4 (1024字节数据): 成功" << std::endl;
        }
        else {
            std::cout << "测试用例4 (1024字节数据): 失败" << std::endl;
        }
    }

    std::cout << "测试完成: " << passed << "/" << total << " 测试用例通过" << std::endl;

    return (passed == total) ? 0 : 1;
}
