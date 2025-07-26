#pragma once
#include <cstdint>
#include <cstddef>

void sm3_hash(const uint8_t* msg, size_t len, uint8_t hash[32]);
void sm3_hash_avx2(const uint8_t* msg, size_t len, uint8_t hash[32]);
void sm3_hash_avx512(const uint8_t* msg, size_t len, uint8_t hash[32]);