#include "sm4.h"

// GFNIָ����Ҫ��S�з������루Ԥ���㣩
static __m128i GFNI_MASK;
static bool is_gfni_init = false;

static void init_gfni() {
    if (is_gfni_init) return;
    // SM4 S�еķ���任����
    uint8_t mask[16] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }; // ʾ��ֵ
    GFNI_MASK = _mm_loadu_si128((__m128i*)mask);
    is_gfni_init = true;
}

// GFNI�Ż���S�б任������GF2P8AFFINEQBָ�
static __m128i SM4_SBOX_gfni(__m128i x) {
    // GF2P8AFFINEQB�����S�еķ�����+���Ա任
    return _mm_gf2p8affineqb_epi64(x, GFNI_MASK, 0); // 0��ʾ�޶�����λ
}

// VPROLD�Ż���L�任��������תָ�
static __m128i SM4_L_vprold(__m128i x) {
    __m128i s2 = _mm_rotl_epi32(x, 2);   // VPROLDָ�����2λ
    __m128i s10 = _mm_rotl_epi32(x, 10); // ����10λ
    __m128i s18 = _mm_rotl_epi32(x, 18); // ����18λ
    __m128i s24 = _mm_rotl_epi32(x, 24); // ����24λ
    return _mm_xor_si128(_mm_xor_si128(x, s2), _mm_xor_si128(s10, _mm_xor_si128(s18, s24)));
}

// GFNI+VPROLD�Ż���T����
static __m128i SM4_T_gfni(__m128i x) {
    __m128i s = SM4_SBOX_gfni(x);
    return SM4_L_vprold(s);
}

// ���ܣ�GFNI+VPROLD�Ż���
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

// ���ܣ�GFNI+VPROLD�Ż���
void sm4::decrypt_gfni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    uint32_t rk_rev[32];
    for (int i = 0; i < 32; ++i) {
        rk_rev[i] = rk[31 - i];
    }
    sm4::encrypt_gfni(input, output, rk_rev);
}