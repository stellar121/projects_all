#pragma once
#include "sm4.h"
#include <cstdint>
#include <cstring>
#include <immintrin.h>

namespace sm4_gcm_opt {
    const size_t BLOCK_SIZE = 16;
    const size_t TAG_SIZE = 16;

    // 利用PCLMULQDQ指令加速GF(2^128)乘法
    __m128i gf_mul_pclmul(const __m128i& a, const __m128i& b);
    // 伽罗瓦加法（异或）
    static inline __m128i gf_add(const __m128i& a, const __m128i& b) {
        return _mm_xor_si128(a, b);
    }

    typedef struct {
        uint32_t rk[32];          // SM4轮密钥
        __m128i auth_key;         // 认证密钥H (128位)
        __m128i nonce_counter;    // 初始计数器 (IV + 0)
        __m128i tag;              // 中间标签值
        size_t ad_len;            // 附加数据总长度（用于最终处理）
        size_t data_len;          // 明文/密文总长度（用于最终处理）
    } Context;

    // 初始化上下文（支持GFNI加速）
    void init(Context& ctx, const uint8_t key[16], const uint8_t iv[], size_t iv_len);

    // 加密更新（AESNI/GFNI加速SM4加密）
    void encrypt_update(Context& ctx, const uint8_t* plaintext, uint8_t* ciphertext, size_t len);

    // 附加数据更新（向量化处理）
    void auth_update(Context& ctx, const uint8_t* adata, size_t len);

    // 完成加密并生成标签（PCLMUL加速）
    void encrypt_final(Context& ctx, uint8_t tag[TAG_SIZE]);

    // 解密更新（复用加密路径，保证一致性）
    void decrypt_update(Context& ctx, const uint8_t* ciphertext, uint8_t* plaintext, size_t len);

    // 验证标签
    bool decrypt_final(Context& ctx, const uint8_t tag[TAG_SIZE]);
}
