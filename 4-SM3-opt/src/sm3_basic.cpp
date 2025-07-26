#include "sm3.h"
#include <cstring>
#include <vector>
#include "sm3_utils.cpp"

void sm3_hash(const uint8_t* msg, size_t len, uint8_t hash[32]) {
    size_t padded_len;
    std::vector<uint8_t> padded(256);
    padding(msg, len, padded.data(), padded_len);

    uint32_t V[8];
    std::memcpy(V, IV, sizeof(V));

    for (size_t i = 0; i < padded_len; i += 64) {
        uint32_t W[68], W1[64];
        for (int j = 0; j < 16; ++j) {
            W[j] = ((uint32_t)padded[i + 4 * j] << 24) |
                ((uint32_t)padded[i + 4 * j + 1] << 16) |
                ((uint32_t)padded[i + 4 * j + 2] << 8) |
                ((uint32_t)padded[i + 4 * j + 3]);
        }
        for (int j = 16; j < 68; ++j)
            W[j] = P1(W[j - 16] ^ W[j - 9] ^ rotate_left(W[j - 3], 15)) ^ rotate_left(W[j - 13], 7) ^ W[j - 6];
        for (int j = 0; j < 64; ++j)
            W1[j] = W[j] ^ W[j + 4];

        uint32_t A = V[0], B = V[1], C = V[2], D = V[3];
        uint32_t E = V[4], F = V[5], G = V[6], H = V[7];

        for (int j = 0; j < 64; ++j) {
            uint32_t T = (j < 16) ? 0x79cc4519 : 0x7a879d8a;
            T = rotate_left(T, j);
            uint32_t SS1 = rotate_left((rotate_left(A, 12) + E + T), 7);
            uint32_t SS2 = SS1 ^ rotate_left(A, 12);
            uint32_t TT1 = FF(A, B, C, j) + D + SS2 + W1[j];
            uint32_t TT2 = GG(E, F, G, j) + H + SS1 + W[j];
            D = C;
            C = rotate_left(B, 9);
            B = A;
            A = TT1;
            H = G;
            G = rotate_left(F, 19);
            F = E;
            E = P0(TT2);
        }

        V[0] ^= A; V[1] ^= B; V[2] ^= C; V[3] ^= D;
        V[4] ^= E; V[5] ^= F; V[6] ^= G; V[7] ^= H;
    }

    for (int i = 0; i < 8; ++i) {
        hash[4 * i] = (V[i] >> 24) & 0xFF;
        hash[4 * i + 1] = (V[i] >> 16) & 0xFF;
        hash[4 * i + 2] = (V[i] >> 8) & 0xFF;
        hash[4 * i + 3] = V[i] & 0xFF;
    }
}