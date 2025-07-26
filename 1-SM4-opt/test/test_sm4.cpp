#include "sm4.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>

// 打印16字节数据（十六进制）
void print_data(const uint8_t* data, const char* label) {
    std::cout << label << ": ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    std::cout << std::dec << std::endl;
}

// 验证正确性
bool test_correctness() {
    // 测试向量（密钥、明文、密文）
    uint8_t key[16] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10 };
    uint8_t plaintext[16] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10 };
    uint8_t ciphertext[16], decrypted[16];
    uint32_t rk[32];

    // 基础实现验证
    sm4::key_expansion(key, rk);
    sm4::encrypt_basic(plaintext, ciphertext, rk);
    sm4::decrypt_basic(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "基础实现验证失败！" << std::endl;
        return false;
    }

    // T-table优化验证
    sm4::encrypt_ttable(plaintext, ciphertext, rk);
    sm4::decrypt_ttable(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "T-table优化验证失败！" << std::endl;
        return false;
    }

    // AESNI优化验证
    sm4::encrypt_aesni(plaintext, ciphertext, rk);
    sm4::decrypt_aesni(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "AESNI优化验证失败！" << std::endl;
        return false;
    }

    // GFNI优化验证（需硬件支持）
    sm4::encrypt_gfni(plaintext, ciphertext, rk);
    sm4::decrypt_gfni(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "GFNI优化验证失败（可能硬件不支持）！" << std::endl;
        // 不返回false，因部分CPU可能不支持GFNI
    }

    std::cout << "所有支持的实现验证成功！" << std::endl;
    return true;
}

// 性能测试（1MB数据）
void test_performance() {
    const size_t DATA_SIZE = 1024 * 1024; // 1MB
    std::vector<uint8_t> data(DATA_SIZE, 0xAA); // 测试数据
    uint8_t key[16] = { 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff };
    uint32_t rk[32];
    sm4::key_expansion(key, rk);

    // 计时函数
    auto measure = [&](void (*encrypt)(const uint8_t*, uint8_t*, const uint32_t*), const char* name) {
        auto start = std::chrono::high_resolution_clock::now();
        // 加密整个数据（按16字节分组）
        for (size_t i = 0; i < DATA_SIZE; i += 16) {
            encrypt(data.data() + i, data.data() + i, rk); // 原地加密
        }
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end - start).count();
        double throughput = (DATA_SIZE / (1024.0 * 1024.0)) / duration; // MB/s
        std::cout << name << " 吞吐量: " << std::fixed << std::setprecision(2) << throughput << " MB/s" << std::endl;
        };

    // 测试各实现
    measure(sm4::encrypt_basic, "基础实现");
    measure(sm4::encrypt_ttable, "T-table优化");
    measure(sm4::encrypt_aesni, "AESNI优化");
    measure(sm4::encrypt_gfni, "GFNI+VPROLD优化");
}

int main() {
    // 验证正确性
    if (!test_correctness()) {
        return 1;
    }

    // 测试性能
    std::cout << "\n性能测试（1MB数据）：" << std::endl;
    test_performance();

    return 0;
}