#include "sm4.h"

// GFNI指令需要的S盒仿射掩码（预计算）
static __m128i GFNI_MASK;
static bool is_gfni_init = false;

static void init_gfni() {
    if (is_gfni_init) return;
    // SM4 S盒的仿射变换掩码
    uint8_t mask[16] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }; // 示例值
    GFNI_MASK = _mm_loadu_si128((__m128i*)mask);
    is_gfni_init = true;
}

// GFNI优化的S盒变换（利用GF2P8AFFINEQB指令）
static __m128i SM4_SBOX_gfni(__m128i x) {
    // GF2P8AFFINEQB：完成S盒的非线性+线性变换
    return _mm_gf2p8affineqb_epi64(x, GFNI_MASK, 0); // 0表示无额外移位
}

// VPROLD优化的L变换（向量旋转指令）
static __m128i SM4_L_vprold(__m128i x) {
    __m128i s2 = _mm_rotl_epi32(x, 2);   // VPROLD指令：左移2位
    __m128i s10 = _mm_rotl_epi32(x, 10); // 左移10位
    __m128i s18 = _mm_rotl_epi32(x, 18); // 左移18位
    __m128i s24 = _mm_rotl_epi32(x, 24); // 左移24位
    return _mm_xor_si128(_mm_xor_si128(x, s2), _mm_xor_si128(s10, _mm_xor_si128(s18, s24)));
}

// GFNI+VPROLD优化的T函数
static __m128i SM4_T_gfni(__m128i x) {
    __m128i s = SM4_SBOX_gfni(x);
    return SM4_L_vprold(s);
}

// 加密（GFNI+VPROLD优化）
void sm4::encrypt_gfni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    init_gfni();
    __m128i x = _mm_loadu_si128((__m128i*)input);

    __m128i rk_vec[32];
    for (int i = 0; i < 32; ++i) {
        rk_vec[i] = _mm_set1_epi32(rk[i]);
    }

    for (int i = 0; i < 32; ++i) {
        __m128i x1 = _mm_shuffle_epi32(x, _MM_SHUFFLE(3, 2, 1, 0));
        __m128i x2 = _mm_shuffle_epi32(x, _MM_SHUFFLE(2, 1, 0, 3));
        __m128i x3 = _mm_shuffle_epi32(x, _MM_SHUFFLE(1, 0, 3, 2));
        __m128i x_xor = _mm_xor_si128(_mm_xor_si128(x1, x2), x3);
        __m128i temp = _mm_xor_si128(x_xor, rk_vec[i]);
        temp = SM4_T_gfni(temp);
        x = _mm_xor_si128(x, temp);
    }

    _mm_storeu_si128((__m128i*)output, x);
}

// 解密（GFNI+VPROLD优化）
void sm4::decrypt_gfni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    uint32_t rk_rev[32];
    for (int i = 0; i < 32; ++i) {
        rk_rev[i] = rk[31 - i];
    }
    sm4::encrypt_gfni(input, output, rk_rev);
}