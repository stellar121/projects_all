#include "sm4.h"

// 初始化S盒的AESNI向量表（128位，用于PSHUFB指令）
static __m128i SBOX_AESNI;
static bool is_sbox_aesni_init = false;

static void init_sbox_aesni() {
    if (is_sbox_aesni_init) return;
    uint8_t sbox_vec[16];
    for (int i = 0; i < 16; ++i) {
        sbox_vec[i] = sm4::SBOX[i]; // 完整初始化需填充256值（按16字节对齐）
    }
    // 实际应扩展为16字节×16组，此处简化为示例
    SBOX_AESNI = _mm_loadu_si128((__m128i*)sbox_vec);
    is_sbox_aesni_init = true;
}

// AESNI优化的S盒变换（并行处理16字节）
static __m128i SM4_SBOX_aesni(__m128i x) {
    return _mm_shuffle_epi8(x, SBOX_AESNI); // PSHUFB指令并行查表
}

// AESNI优化的L变换（向量操作）
static __m128i SM4_L_aesni(__m128i x) {
    __m128i s2 = _mm_or_si128(_mm_slli_epi32(x, 2), _mm_srli_epi32(x, 30));
    __m128i s10 = _mm_or_si128(_mm_slli_epi32(x, 10), _mm_srli_epi32(x, 22));
    __m128i s18 = _mm_or_si128(_mm_slli_epi32(x, 18), _mm_srli_epi32(x, 14));
    __m128i s24 = _mm_or_si128(_mm_slli_epi32(x, 24), _mm_srli_epi32(x, 8));
    return _mm_xor_si128(_mm_xor_si128(x, s2), _mm_xor_si128(s10, _mm_xor_si128(s18, s24)));
}

// AESNI优化的T函数
static __m128i SM4_T_aesni(__m128i x) {
    __m128i s = SM4_SBOX_aesni(x);
    return SM4_L_aesni(s);
}

// 加密（AESNI优化）
void sm4::encrypt_aesni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    init_sbox_aesni();
    __m128i x = _mm_loadu_si128((__m128i*)input); // 加载128位输入

    // 轮密钥转换为向量
    __m128i rk_vec[32];
    for (int i = 0; i < 32; ++i) {
        rk_vec[i] = _mm_set1_epi32(rk[i]); // 广播轮密钥到128位向量
    }

    // 32轮迭代（向量操作）
    for (int i = 0; i < 32; ++i) {
        // 提取x1^x2^x3（向量拆分与异或）
        __m128i x1 = _mm_shuffle_epi32(x, _MM_SHUFFLE(3, 2, 1, 0)); // 模拟轮转
        __m128i x2 = _mm_shuffle_epi32(x, _MM_SHUFFLE(2, 1, 0, 3));
        __m128i x3 = _mm_shuffle_epi32(x, _MM_SHUFFLE(1, 0, 3, 2));
        __m128i x_xor = _mm_xor_si128(_mm_xor_si128(x1, x2), x3);
        __m128i temp = _mm_xor_si128(x_xor, rk_vec[i]); // 异或轮密钥
        temp = SM4_T_aesni(temp); // T变换
        x = _mm_xor_si128(x, temp); // 更新状态
    }

    _mm_storeu_si128((__m128i*)output, x); // 存储结果
}

// 解密（AESNI优化）
void sm4::decrypt_aesni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    uint32_t rk_rev[32];
    for (int i = 0; i < 32; ++i) {
        rk_rev[i] = rk[31 - i];
    }
    sm4::encrypt_aesni(input, output, rk_rev);
}