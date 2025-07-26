#include "sm3.h"
#include <cstring>

namespace {
    constexpr uint32_t IV[8] = {
        0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
        0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
    };

    uint32_t rotate_left(uint32_t x, int n) {
        return (x << n) | (x >> (32 - n));
    }

    uint32_t P0(uint32_t x) {
        return x ^ rotate_left(x, 9) ^ rotate_left(x, 17);
    }

    uint32_t P1(uint32_t x) {
        return x ^ rotate_left(x, 15) ^ rotate_left(x, 23);
    }

    uint32_t FF(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j < 16) ? (x ^ y ^ z) : ((x & y) | (x & z) | (y & z));
    }

    uint32_t GG(uint32_t x, uint32_t y, uint32_t z, int j) {
        return (j < 16) ? (x ^ y ^ z) : ((x & y) | ((~x) & z));
    }

    void padding(const uint8_t* msg, size_t len, uint8_t* padded, size_t& padded_len) {
        size_t bit_len = len * 8;
        padded_len = ((len + 9 + 63) / 64) * 64;
        std::memset(padded, 0, padded_len);
        std::memcpy(padded, msg, len);
        padded[len] = 0x80;
        for (int i = 0; i < 8; ++i)
            padded[padded_len - 1 - i] = (bit_len >> (8 * i)) & 0xFF;
    }
}