#include "sm4_gcm_optimized.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cassert>

// 打印十六进制数据
void print_hex(const uint8_t* data, size_t len, const std::string& label) {
    std::cout << label << ": ";
    for (size_t i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    }
    std::cout << std::dec << std::endl;
}

// 功能验证：与基础实现对比
bool test_functionality() {
    // 测试向量（与基础实现保持一致）
    const uint8_t key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    const uint8_t iv[12] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b };
    const uint8_t plaintext[] = "Optimized SM4-GCM Test";
    const size_t plaintext_len = strlen((const char*)plaintext);
    const uint8_t adata[] = "Authenticated Data";
    const size_t adata_len = strlen((const char*)adata);

    // 加密
    sm4_gcm_opt::Context enc_ctx;
    uint8_t ciphertext[plaintext_len];
    uint8_t tag[sm4_gcm_opt::TAG_SIZE];
    sm4_gcm_opt::init(enc_ctx, key, iv, sizeof(iv));
    sm4_gcm_opt::auth_update(enc_ctx, adata, adata_len);
    sm4_gcm_opt::encrypt_update(enc_ctx, plaintext, ciphertext, plaintext_len);
    sm4_gcm_opt::encrypt_final(enc_ctx, tag);

    // 解密验证
    sm4_gcm_opt::Context dec_ctx;
    uint8_t decrypted[plaintext_len];
    sm4_gcm_opt::init(dec_ctx, key, iv, sizeof(iv));
    sm4_gcm_opt::auth_update(dec_ctx, adata, adata_len);
    sm4_gcm_opt::decrypt_update(dec_ctx, ciphertext, decrypted, plaintext_len);
    bool tag_ok = sm4_gcm_opt::decrypt_final(dec_ctx, tag);
    bool data_ok = (memcmp(decrypted, plaintext, plaintext_len) == 0);

    // 打印结果
    print_hex(plaintext, plaintext_len, "原始明文");
    print_hex(decrypted, plaintext_len, "解密结果");
    print_hex(tag, sm4_gcm_opt::TAG_SIZE, "认证标签");

    return tag_ok && data_ok;
}

// 性能测试：对比优化前后的吞吐量
void test_performance() {
    const size_t DATA_SIZE = 1024 * 1024 * 10;  // 10MB
    uint8_t* plaintext = new uint8_t[DATA_SIZE];
    uint8_t* ciphertext = new uint8_t[DATA_SIZE];
    uint8_t* adata = new uint8_t[DATA_SIZE / 10];  // 附加数据1MB
    uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                       0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t iv[12] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b };
    uint8_t tag[sm4_gcm_opt::TAG_SIZE];

    // 初始化随机数据
    for (size_t i = 0; i < DATA_SIZE; ++i) plaintext[i] = rand() % 256;
    for (size_t i = 0; i < DATA_SIZE / 10; ++i) adata[i] = rand() % 256;

    // 测试优化后的加密性能
    sm4_gcm_opt::Context ctx;
    auto start = std::chrono::high_resolution_clock::now();

    sm4_gcm_opt::init(ctx, key, iv, sizeof(iv));
    sm4_gcm_opt::auth_update(ctx, adata, DATA_SIZE / 10);
    sm4_gcm_opt::encrypt_update(ctx, plaintext, ciphertext, DATA_SIZE);
    sm4_gcm_opt::encrypt_final(ctx, tag);

    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    double throughput = (DATA_SIZE / (1024.0 * 1024.0)) / duration;  // MB/s

    std::cout << "\n优化后性能测试：" << std::endl;
    std::cout << "数据大小: " << DATA_SIZE / (1024 * 1024) << "MB" << std::endl;
    std::cout << "耗时: " << std::fixed << std::setprecision(3) << duration << "秒" << std::endl;
    std::cout << "吞吐量: " << std::fixed << std::setprecision(2) << throughput << " MB/s" << std::endl;

    delete[] plaintext;
    delete[] ciphertext;
    delete[] adata;
}

int main() {
    std::cout << "SM4-GCM 优化实现测试" << std::endl;
    std::cout << "---------------------" << std::endl;

    // 功能验证
    bool func_ok = test_functionality();
    std::cout << "功能验证: " << (func_ok ? "通过" : "失败") << std::endl;

    // 性能测试
    if (func_ok) {
        test_performance();
    }

    return func_ok ? 0 : 1;
}
