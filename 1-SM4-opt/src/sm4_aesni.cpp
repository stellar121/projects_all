#include "sm4.h"

// ��ʼ��S�е�AESNI������128λ������PSHUFBָ�
static __m128i SBOX_AESNI;
static bool is_sbox_aesni_init = false;

static void init_sbox_aesni() {
    if (is_sbox_aesni_init) return;
    uint8_t sbox_vec[16];
    for (int i = 0; i < 16; ++i) {
        sbox_vec[i] = sm4::SBOX[i]; // ������ʼ�������256ֵ����16�ֽڶ��룩
    }
    // ʵ��Ӧ��չΪ16�ֽڡ�16�飬�˴���Ϊʾ��
    SBOX_AESNI = _mm_loadu_si128((__m128i*)sbox_vec);
    is_sbox_aesni_init = true;
}

// AESNI�Ż���S�б任�����д���16�ֽڣ�
static __m128i SM4_SBOX_aesni(__m128i x) {
    return _mm_shuffle_epi8(x, SBOX_AESNI); // PSHUFBָ��в��
}

// AESNI�Ż���L�任������������
static __m128i SM4_L_aesni(__m128i x) {
    __m128i s2 = _mm_or_si128(_mm_slli_epi32(x, 2), _mm_srli_epi32(x, 30));
    __m128i s10 = _mm_or_si128(_mm_slli_epi32(x, 10), _mm_srli_epi32(x, 22));
    __m128i s18 = _mm_or_si128(_mm_slli_epi32(x, 18), _mm_srli_epi32(x, 14));
    __m128i s24 = _mm_or_si128(_mm_slli_epi32(x, 24), _mm_srli_epi32(x, 8));
    return _mm_xor_si128(_mm_xor_si128(x, s2), _mm_xor_si128(s10, _mm_xor_si128(s18, s24)));
}

// AESNI�Ż���T����
static __m128i SM4_T_aesni(__m128i x) {
    __m128i s = SM4_SBOX_aesni(x);
    return SM4_L_aesni(s);
}

// ���ܣ�AESNI�Ż���
void sm4::encrypt_aesni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    init_sbox_aesni();
    __m128i x = _mm_loadu_si128((__m128i*)input); // ����128λ����

    // ����Կת��Ϊ����
    __m128i rk_vec[32];
    for (int i = 0; i < 32; ++i) {
        rk_vec[i] = _mm_set1_epi32(rk[i]); // �㲥����Կ��128λ����
    }

    // 32�ֵ���������������
    for (int i = 0; i < 32; ++i) {
        // ��ȡx1^x2^x3��������������
        __m128i x1 = _mm_shuffle_epi32(x, _MM_SHUFFLE(3, 2, 1, 0)); // ģ����ת
        __m128i x2 = _mm_shuffle_epi32(x, _MM_SHUFFLE(2, 1, 0, 3));
        __m128i x3 = _mm_shuffle_epi32(x, _MM_SHUFFLE(1, 0, 3, 2));
        __m128i x_xor = _mm_xor_si128(_mm_xor_si128(x1, x2), x3);
        __m128i temp = _mm_xor_si128(x_xor, rk_vec[i]); // �������Կ
        temp = SM4_T_aesni(temp); // T�任
        x = _mm_xor_si128(x, temp); // ����״̬
    }

    _mm_storeu_si128((__m128i*)output, x); // �洢���
}

// ���ܣ�AESNI�Ż���
void sm4::decrypt_aesni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    uint32_t rk_rev[32];
    for (int i = 0; i < 32; ++i) {
        rk_rev[i] = rk[31 - i];
    }
    sm4::encrypt_aesni(input, output, rk_rev);
}