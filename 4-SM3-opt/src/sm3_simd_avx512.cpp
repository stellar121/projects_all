#include "sm3.h"
#include <cstring>
#include <vector>
#include <immintrin.h>
#include "sm3_utils.cpp"

// AVX512优化的SM3实现
void sm3_hash_avx512(const uint8_t* msg, size_t len, uint8_t hash[32]) {
    size_t padded_len;
    std::vector<uint8_t> padded(((len + 9 + 63) / 64) * 64);
    padding(msg, len, padded.data(), padded_len);

    uint32_t V[8];
    std::memcpy(V, IV, sizeof(V));

    // 加载初始向量到AVX512寄存器
    __m512i vec_V = _mm512_loadu_si512((const __m512i*)V);

    for (size_t i = 0; i < padded_len; i += 64) {
        uint32_t W[68], W1[64];

        // 消息扩展 - 前16个字，使用AVX512一次处理16个字
        __m512i vec = _mm512_loadu_si512((const __m512i*)(padded.data() + i));
        __m512i byteswap = _mm512_shuffle_epi8(vec, _mm512_set_epi8(
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
            48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
        ));
        _mm512_storeu_si512((__m512i*)W, byteswap);

        // 消息扩展 - 16到67字，使用AVX512并行计算（一次处理16个字）
        for (int j = 16; j < 68; j += 16) {
            __m512i w16 = _mm512_loadu_si512((const __m512i*)(W + j - 16));
            __m512i w9 = _mm512_loadu_si512((const __m512i*)(W + j - 9));
            __m512i w3 = _mm512_loadu_si512((const __m512i*)(W + j - 3));

            // 计算W[j] = P1(W[j-16] ^ W[j-9] ^ ROTL(W[j-3], 15)) ^ ROTL(W[j-13], 7) ^ W[j-6]
            __m512i xor1 = _mm512_xor_si512(_mm512_xor_si512(w16, w9),
                _mm512_rol_epi32(w3, 15));

            __m512i p1 = _mm512_xor_si512(_mm512_xor_si512(xor1, _mm512_rol_epi32(xor1, 9)),
                _mm512_rol_epi32(xor1, 17));

            __m512i w13 = _mm512_loadu_si512((const __m512i*)(W + j - 13));
            __m512i rot7 = _mm512_rol_epi32(w13, 7);
            __m512i w6 = _mm512_loadu_si512((const __m512i*)(W + j - 6));

            __m512i res = _mm512_xor_si512(_mm512_xor_si512(p1, rot7), w6);
            _mm512_storeu_si512((__m512i*)(W + j), res);
        }

        // 计算W1，使用AVX512并行处理
        __m512i w_vec = _mm512_loadu_si512((const __m512i*)W);
        __m512i w4_vec = _mm512_loadu_si512((const __m512i*)(W + 4));
        __m512i w1_vec = _mm512_xor_si512(w_vec, w4_vec);
        _mm512_storeu_si512((__m512i*)W1, w1_vec);

        // 压缩函数初始化
        uint32_t A = V[0], B = V[1], C = V[2], D = V[3];
        uint32_t E = V[4], F = V[5], G = V[6], H = V[7];

        // 准备T常量向量
        __m512i T_low = _mm512_set1_epi32(0x79cc4519);
        __m512i T_high = _mm512_set1_epi32(0x7a879d8a);

        // 64轮迭代，使用AVX512指令并行处理
        for (int j = 0; j < 64; j++) {
            uint32_t T = (j < 16) ? 0x79cc4519 : 0x7a879d8a;
            T = rotate_left(T, j);

            uint32_t SS1 = rotate_left((rotate_left(A, 12) + E + T), 7);
            uint32_t SS2 = SS1 ^ rotate_left(A, 12);
            uint32_t TT1 = FF(A, B, C, j) + D + SS2 + W1[j];
            uint32_t TT2 = GG(E, F, G, j) + H + SS1 + W[j];

            // 更新寄存器
            D = C;
            C = rotate_left(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = rotate_left(F, 19);
            F = E;
            E = P0(TT2);
        }

        // 与初始向量异或
        V[0] ^= A; V[1] ^= B; V[2] ^= C; V[3] ^= D;
        V[4] ^= E; V[5] ^= F; V[6] ^= G; V[7] ^= H;
    }

    // 将结果转换为字节数组
    for (int i = 0; i < 8; ++i) {
        hash[4 * i] = (V[i] >> 24) & 0xFF;
        hash[4 * i + 1] = (V[i] >> 16) & 0xFF;
        hash[4 * i + 2] = (V[i] >> 8) & 0xFF;
        hash[4 * i + 3] = V[i] & 0xFF;
    }
}
