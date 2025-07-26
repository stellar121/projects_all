#include "sm3.h"
#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>

// 将哈希值（大端字节序）转换为内部状态（32位字）
void hash_to_state(const uint8_t hash[32], uint32_t state[8]) {
    for (int i = 0; i < 8; ++i) {
        // 哈希值是大端存储，转换为32位无符号整数
        state[i] = ((uint32_t)hash[4 * i] << 24) |
            ((uint32_t)hash[4 * i + 1] << 16) |
            ((uint32_t)hash[4 * i + 2] << 8) |
            (uint32_t)hash[4 * i + 3];
    }
}

// 将内部状态转换为哈希值（大端字节序）
void state_to_hash(const uint32_t state[8], uint8_t hash[32]) {
    for (int i = 0; i < 8; ++i) {
        hash[4 * i] = (state[i] >> 24) & 0xFF;
        hash[4 * i + 1] = (state[i] >> 16) & 0xFF;
        hash[4 * i + 2] = (state[i] >> 8) & 0xFF;
        hash[4 * i + 3] = state[i] & 0xFF;
    }
}

// 打印哈希值
void print_hash(const uint8_t hash[32], const char* label) {
    std::cout << label << ": ";
    for (int i = 0; i < 32; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    std::cout << std::dec << std::endl;
}

int main() {
    // 1. 原始消息
    const uint8_t original_msg[] = "I like sm3!";
    size_t original_len = strlen((const char*)original_msg);
    size_t original_bit_len = original_len * 8;  // 原始消息的比特长度
    uint8_t original_hash[32];

    // 计算原始消息的哈希
    sm3_hash(original_msg, original_len, original_hash);
    print_hash(original_hash, "原始消息哈希");

    // 2. 攻击者已知：原始哈希值 + 原始消息长度，构造扩展内容
    const uint8_t extension[] = " And this is the extension.";
    size_t extension_len = strlen((const char*)extension);

    // 3. 攻击者从哈希值恢复内部状态
    uint32_t attacker_state[8];
    hash_to_state(original_hash, attacker_state);  // 正确处理字节序

    // 4. 攻击者计算扩展消息的哈希（不依赖原始消息）
    sm3_hash_with_state(extension, extension_len, original_bit_len, attacker_state);
    uint8_t attacker_hash[32];
    state_to_hash(attacker_state, attacker_hash);
    print_hash(attacker_hash, "攻击者构造的哈希");

    // 5. 真实值：原始消息 + 原始填充 + 扩展内容 的哈希
    // 构造真实扩展消息 M' = M || padding(M) || extension
    std::vector<uint8_t> original_padded(((original_len + 9 + 63) / 64) * 64);
    size_t original_padded_len;
    // 临时调用内部填充函数计算原始消息的填充
    {
        namespace internal = ::_;
        internal::padding(original_msg, original_len, original_padded.data(), original_padded_len, original_bit_len);
    }

    // 拼接原始填充后的消息 + 扩展内容
    std::vector<uint8_t> real_extended_msg;
    real_extended_msg.insert(real_extended_msg.end(),
        original_padded.begin(), original_padded.end());
    real_extended_msg.insert(real_extended_msg.end(),
        extension, extension + extension_len);

    // 计算真实哈希
    uint8_t real_hash[32];
    sm3_hash(real_extended_msg.data(), real_extended_msg.size(), real_hash);
    print_hash(real_hash, "真实扩展消息哈希");

    // 6. 验证攻击是否成功
    if (memcmp(attacker_hash, real_hash, 32) == 0) {
        std::cout << "攻击成功：构造的哈希与真实哈希一致！" << std::endl;
    }
    else {
        std::cout << "攻击失败：哈希不一致！" << std::endl;
    }

    return 0;
}
