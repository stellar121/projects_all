#include "sm3.h"
#include <cstring>
#include <vector>
#include <immintrin.h>
#include "sm3_utils.cpp"

// AVX2优化的SM3实现
void sm3_hash_avx2(const uint8_t* msg, size_t len, uint8_t hash[32]) {
    size_t padded_len;
    std::vector<uint8_t> padded(((len + 9 + 63) / 64) * 64);
    padding(msg, len, padded.data(), padded_len);

    uint32_t V[8];
    std::memcpy(V, IV, sizeof(V));

    // 加载初始向量到AVX2寄存器
    __m256i vec_V = _mm256_loadu_si256((const __m256i*)V);

    for (size_t i = 0; i < padded_len; i += 64) {
        uint32_t W[68], W1[64];

        // 消息扩展 - 前16个字
        __m256i vec_w0, vec_w1;
        for (int j = 0; j < 16; j += 8) {
            // 一次加载8个字节到向量寄存器，转换为大端格式
            __m256i vec = _mm256_loadu_si256((const __m256i*)(padded.data() + i + j * 4));
            __m256i byteswap = _mm256_shuffle_epi8(vec, _mm256_set_epi8(
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
            ));
            _mm256_storeu_si256((__m256i*)(W + j), byteswap);
        }

        // 消息扩展 - 16到67字，使用AVX2并行计算
        for (int j = 16; j < 68; j += 2) {
            __m256i w16 = _mm256_loadu_si256((const __m256i*)(W + j - 16));
            __m256i w9 = _mm256_loadu_si256((const __m256i*)(W + j - 9));
            __m256i w3 = _mm256_loadu_si256((const __m256i*)(W + j - 3));

            // 计算W[j] = P1(W[j-16] ^ W[j-9] ^ ROTL(W[j-3], 15)) ^ ROTL(W[j-13], 7) ^ W[j-6]
            __m256i xor1 = _mm256_xor_si256(_mm256_xor_si256(w16, w9),
                _mm256_or_si256(_mm256_slli_epi32(w3, 15), _mm256_srli_epi32(w3, 17)));

            __m256i p1 = _mm256_xor_si256(_mm256_xor_si256(xor1,
                _mm256_or_si256(_mm256_slli_epi32(xor1, 9), _mm256_srli_epi32(xor1, 23))),
                _mm256_or_si256(_mm256_slli_epi32(xor1, 17), _mm256_srli_epi32(xor1, 15)));

            __m256i w13 = _mm256_loadu_si256((const __m256i*)(W + j - 13));
            __m256i rot7 = _mm256_or_si256(_mm256_slli_epi32(w13, 7), _mm256_srli_epi32(w13, 25));
            __m256i w6 = _mm256_loadu_si256((const __m256i*)(W + j - 6));

            __m256i res = _mm256_xor_si256(_mm256_xor_si256(p1, rot7), w6);
            _mm256_storeu_si256((__m256i*)(W + j), res);
        }

        // 计算W1
        for (int j = 0; j < 64; j++) {
            W1[j] = W[j] ^ W[j + 4];
        }

        // 压缩函数初始化
        uint32_t A = V[0], B = V[1], C = V[2], D = V[3];
        uint32_t E = V[4], F = V[5], G = V[6], H = V[7];

        // 64轮迭代，前16轮和后48轮使用不同的常量
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
