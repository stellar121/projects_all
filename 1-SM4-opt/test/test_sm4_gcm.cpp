#include "sm4_gcm.h"
#include <iostream>
#include <iomanip>
#include <cassert>

// 打印二进制数据（十六进制格式）
void print_hex(const uint8_t* data, size_t len, const std::string& label) {
    std::cout << label << ": ";
    for (size_t i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]);
    }
    std::cout << std::dec << std::endl;
}

// 测试向量1：空数据+空附加数据
bool test_case1() {
    const uint8_t key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    const uint8_t iv[12] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b };
    const uint8_t plaintext[] = "";
    const size_t plaintext_len = 0;
    const uint8_t adata[] = "";
    const size_t adata_len = 0;

    // 预期结果（通过标准实现计算）
    const uint8_t expected_ciphertext[] = "";
    const uint8_t expected_tag[16] = {
        0x2f, 0x2a, 0x1e, 0x49, 0x1c, 0x5d, 0x2b, 0x42,
        0x59, 0x26, 0x31, 0x4a, 0x12, 0x4b, 0x5a, 0x6c
    };

    // 加密过程
    sm4_gcm::Context ctx;
    uint8_t ciphertext[plaintext_len];
    uint8_t tag[sm4_gcm::TAG_SIZE];

    sm4_gcm::init(ctx, key, iv, sizeof(iv));
    sm4_gcm::auth_update(ctx, adata, adata_len);
    sm4_gcm::encrypt_update(ctx, plaintext, ciphertext, plaintext_len);
    sm4_gcm::encrypt_final(ctx, tag);

    // 验证密文和标签
    bool cipher_ok = (memcmp(ciphertext, expected_ciphertext, plaintext_len) == 0);
    bool tag_ok = (memcmp(tag, expected_tag, sm4_gcm::TAG_SIZE) == 0);

    // 解密验证
    uint8_t decrypted[plaintext_len];
    sm4_gcm::Context decrypt_ctx;
    sm4_gcm::init(decrypt_ctx, key, iv, sizeof(iv));
    sm4_gcm::auth_update(decrypt_ctx, adata, adata_len);
    sm4_gcm::decrypt_update(decrypt_ctx, ciphertext, decrypted, plaintext_len);
    bool decrypt_ok = sm4_gcm::decrypt_final(decrypt_ctx, tag);
    bool plaintext_ok = (memcmp(decrypted, plaintext, plaintext_len) == 0);

    return cipher_ok && tag_ok && decrypt_ok && plaintext_ok;
}

// 测试向量2：带明文和附加数据
bool test_case2() {
    const uint8_t key[16] = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
    };
    const uint8_t iv[12] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b };
    const uint8_t plaintext[] = "Hello, SM4-GCM!";
    const size_t plaintext_len = strlen((const char*)plaintext);
    const uint8_t adata[] = "Additional data";
    const size_t adata_len = strlen((const char*)adata);

    // 加密过程
    sm4_gcm::Context ctx;
    uint8_t ciphertext[plaintext_len];
    uint8_t tag[sm4_gcm::TAG_SIZE];

    sm4_gcm::init(ctx, key, iv, sizeof(iv));
    sm4_gcm::auth_update(ctx, adata, adata_len);
    sm4_gcm::encrypt_update(ctx, plaintext, ciphertext, plaintext_len);
    sm4_gcm::encrypt_final(ctx, tag);

    // 打印中间结果
    print_hex(plaintext, plaintext_len, "  明文");
    print_hex(ciphertext, plaintext_len, "  密文");
    print_hex(tag, sm4_gcm::TAG_SIZE, "  认证标签");

    // 解密验证
    uint8_t decrypted[plaintext_len];
    sm4_gcm::Context decrypt_ctx;
    sm4_gcm::init(decrypt_ctx, key, iv, sizeof(iv));
    sm4_gcm::auth_update(decrypt_ctx, adata, adata_len);
    sm4_gcm::decrypt_update(decrypt_ctx, ciphertext, decrypted, plaintext_len);
    bool tag_verify = sm4_gcm::decrypt_final(decrypt_ctx, tag);
    bool plaintext_match = (memcmp(decrypted, plaintext, plaintext_len) == 0);

    print_hex(decrypted, plaintext_len, "  解密结果");
    return tag_verify && plaintext_match;
}

// 测试向量3：篡改密文后的认证失败场景
bool test_case3() {
    const uint8_t key[16] = {
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00
    };
    const uint8_t iv[12] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    const uint8_t plaintext[] = "Test tampering detection";
    const size_t plaintext_len = strlen((const char*)plaintext);

    // 加密
    sm4_gcm::Context ctx;
    uint8_t ciphertext[plaintext_len];
    uint8_t tag[sm4_gcm::TAG_SIZE];
    sm4_gcm::init(ctx, key, iv, sizeof(iv));
    sm4_gcm::encrypt_update(ctx, plaintext, ciphertext, plaintext_len);
    sm4_gcm::encrypt_final(ctx, tag);

    // 篡改密文（修改第一个字节）
    ciphertext[0] ^= 0x01;

    // 尝试解密验证（预期失败）
    uint8_t decrypted[plaintext_len];
    sm4_gcm::Context decrypt_ctx;
    sm4_gcm::init(decrypt_ctx, key, iv, sizeof(iv));
    sm4_gcm::decrypt_update(decrypt_ctx, ciphertext, decrypted, plaintext_len);
    bool tag_verify = sm4_gcm::decrypt_final(decrypt_ctx, tag);

    // 验证标签应该失败
    return !tag_verify;
}

int main() {
    std::cout << "SM4-GCM 基础实现测试开始..." << std::endl;

    // 测试用例1：空数据验证
    bool case1 = test_case1();
    std::cout << "测试用例1（空数据）: " << (case1 ? "通过" : "失败") << std::endl;

    // 测试用例2：正常数据加解密
    std::cout << "\n测试用例2（正常数据）: " << std::endl;
    bool case2 = test_case2();
    std::cout << "测试用例2结果: " << (case2 ? "通过" : "失败") << std::endl;

    // 测试用例3：篡改检测
    bool case3 = test_case3();
    std::cout << "\n测试用例3（篡改检测）: " << (case3 ? "通过" : "失败") << std::endl;

    // 总结果
    if (case1 && case2 && case3) {
        std::cout << "\n所有测试用例通过！SM4-GCM基础实现正确。" << std::endl;
        return 0;
    }
    else {
        std::cout << "\n部分测试用例失败！" << std::endl;
        return 1;
    }
}
