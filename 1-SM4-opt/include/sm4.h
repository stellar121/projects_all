#pragma once
#include <cstdint>
#include <cstring>
#include <immintrin.h>

// SM4算法常量（完整定义）
namespace sm4 {
    // 完整S盒（8位输入→8位输出）
    extern const uint8_t SBOX[256];

    // 轮常量CK（32个，用于密钥扩展）
    extern const uint32_t CK[32];

    // 密钥扩展：输入128位密钥（16字节），输出32轮密钥（32×4字节）
    void key_expansion(const uint8_t key[16], uint32_t rk[32]);

    // 基础实现：加密/解密
    void encrypt_basic(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
    void decrypt_basic(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);

    // T-table优化实现
    void encrypt_ttable(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
    void decrypt_ttable(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);

    // AESNI指令集优化实现
    void encrypt_aesni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
    void decrypt_aesni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);

    // GFNI+VPROLD指令集优化实现
    void encrypt_gfni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
    void decrypt_gfni(const uint8_t input[16], uint8_t output[16], const uint32_t rk[32]);
}