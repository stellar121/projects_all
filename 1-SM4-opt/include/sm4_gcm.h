#pragma once
#include "sm4.h"  // 包含 SM4 基础实现与优化代码
#include <cstdint>
#include <cstring>
#include <immintrin.h>

namespace sm4_gcm {
    // GCM 相关常量
    const size_t BLOCK_SIZE = 16;  // 128 位分组
    const size_t TAG_SIZE = 16;    // 认证标签长度（128 位）

    // 伽罗瓦域乘法（GF(2^128)）基础函数
    __m128i gf_mul(const __m128i& a, const __m128i& b);
    __m128i gf_add(const __m128i& a, const __m128i& b);

    // SM4 - GCM 上下文
    typedef struct {
        sm4::Key rk;           // SM4 轮密钥（32 轮）
        __m128i auth_key;      // 认证密钥 H（128 位）
        __m128i nonce_counter; // 初始计数器（IV + 0）
        __m128i tag;           // 中间标签值
    } Context;

    // 初始化 GCM 上下文
    void init(Context& ctx, const uint8_t key[sm4::KEY_SIZE], const uint8_t iv[], size_t iv_len);

    // 加密并更新认证标签（处理明文与附加数据）
    void encrypt_update(Context& ctx, const uint8_t* plaintext, uint8_t* ciphertext, size_t len);
    void auth_update(Context& ctx, const uint8_t* adata, size_t len);

    // 完成加密并生成认证标签
    void encrypt_final(Context& ctx, uint8_t tag[TAG_SIZE]);

    // 解密并验证认证标签
    bool decrypt_update(Context& ctx, const uint8_t* ciphertext, uint8_t* plaintext, size_t len);
    bool decrypt_final(Context& ctx, const uint8_t tag[TAG_SIZE]);
}
