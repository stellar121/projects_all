#include "sm3.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <random>

// ���������������
std::vector<uint8_t> generate_test_data(size_t size) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    // ��ģ�������Ϊint
    std::uniform_int_distribution<int> dist(0, 255);

    for (auto& byte : data) {
        // �����������ת��Ϊuint8_t
        byte = static_cast<uint8_t>(dist(gen));
    }
    return data;
}
// ��׼���Ժ���
template <typename Func>
double benchmark(Func hash_func, const uint8_t* data, size_t size, int iterations) {
    uint8_t hash[32];
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        hash_func(data, size, hash);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // ���������� (MB/s)
    double total_bytes = static_cast<double>(size) * iterations;
    return (total_bytes / (1024 * 1024)) / elapsed.count();
}

