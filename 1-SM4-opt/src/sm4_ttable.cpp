#include "sm4.h"

// Ԥ����S�в���Ż�����Ч�ʣ�
static const uint8_t SBOX_TABLE[256] = sm4::SBOX;

// �Ż���T������T-table���
static uint32_t SM4_T_ttable(uint32_t x) {
    // �ֽڼ��������λ���㣩
    uint32_t s = (SBOX_TABLE[(x >> 24) & 0xFF] << 24) |
        (SBOX_TABLE[(x >> 16) & 0xFF] << 16) |
        (SBOX_TABLE[(x >> 8) & 0xFF] << 8) |
        SBOX_TABLE[x & 0xFF];
    // �Ż���L�任���ϲ���λ������
    return s ^
        ((s << 2) | (s >> 30)) ^
        ((s << 10) | (s >> 22)) ^
        ((s << 18) | (s >> 14)) ^
        ((s << 24) | (s >> 8));
}

// ��Կ��չ��T-table�Ż���
static void key_expansion_ttable(const uint8_t key[16], uint32_t rk[32]) {
    uint32_t k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = (key[4 * i] << 24) | (key[4 * i + 1] << 16) | (key[4 * i + 2] << 8) | key[4 * i + 3];
    }
    for (int i = 0; i < 32; ++i) {
        uint32_t temp = k[i % 4] ^ SM4_T_ttable(k[(i + 1) % 4] ^ k[(i + 2) % 4] ^ k[(i + 3) % 4] ^ sm4::CK[i]);
        rk[i] = temp;
        k[i % 4] = temp;
    }
}

// ���ܣ�T-table�Ż���
void sm4::encrypt_ttable(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    uint32_t x[4];
    for (int i = 0; i < 4; ++i) {
        x[i] = (input[4 * i] << 24) | (input[4 * i + 1] << 16) | (input[4 * i + 2] << 8) | input[4 * i + 3];
    }
    for (int i = 0; i < 32; ++i) {
        uint32_t temp = x[0] ^ SM4_T_ttable(x[1] ^ x[2] ^ x[3] ^ rk[i]);
        x[0] = x[1];
        x[1] = x[2];
        x[2] = x[3];
        x[3] = temp;
    }
    uint32_t out[4] = { x[3], x[2], x[1], x[0] };
    for (int i = 0; i < 4; ++i) {
        output[4 * i] = (out[i] >> 24) & 0xFF;
        output[4 * i + 1] = (out[i] >> 16) & 0xFF;
        output[4 * i + 2] = (out[i] >> 8) & 0xFF;
        output[4 * i + 3] = out[i] & 0xFF;
    }
}

// ���ܣ�T-table�Ż���
void sm4::decrypt_ttable(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]) {
    uint32_t rk_rev[32];
    for (int i = 0; i < 32; ++i) {
        rk_rev[i] = rk[31 - i];
    }
    sm4::encrypt_ttable(input, output, rk_rev);
}