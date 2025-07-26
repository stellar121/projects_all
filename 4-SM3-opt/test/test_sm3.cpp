#include "sm3.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

// ��������������ϣ���ת��Ϊʮ�������ַ���
std::string hash_to_string(const uint8_t hash[32]) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 32; ++i) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}

// ���Ժ������Ƚϲ�ͬʵ�ֵĽ���Ƿ�һ������ȷ
bool test_case(const uint8_t* msg, size_t len, const std::string& expected) {
    uint8_t hash_basic[32];
    uint8_t hash_avx2[32];
    uint8_t hash_avx512[32];

    // ���㲻ͬʵ�ֵĹ�ϣֵ
    sm3_hash(msg, len, hash_basic);
    std::string basic_str = hash_to_string(hash_basic);

    // ������ʵ���Ƿ���ȷ
    if (basic_str != expected) {
        std::cerr << "����ʵ�ֲ���ʧ��: Ԥ�� " << expected << ", ʵ�� " << basic_str << std::endl;
        return false;
    }

    // ���AVX2ʵ��
    sm3_hash_avx2(msg, len, hash_avx2);
    std::string avx2_str = hash_to_string(hash_avx2);
    if (avx2_str != expected) {
        std::cerr << "AVX2ʵ�ֲ���ʧ��: Ԥ�� " << expected << ", ʵ�� " << avx2_str << std::endl;
        return false;
    }

    // ���AVX512ʵ��
    sm3_hash_avx512(msg, len, hash_avx512);
    std::string avx512_str = hash_to_string(hash_avx512);
    if (avx512_str != expected) {
        std::cerr << "AVX512ʵ�ֲ���ʧ��: Ԥ�� " << expected << ", ʵ�� " << avx512_str << std::endl;
        return false;
    }

    return true;
}

int main() {
    int passed = 0;
    int total = 0;

    std::cout << "��ʼSM3��ϣ�㷨����..." << std::endl;

    // ��������1: ���ַ���
    {
        total++;
        const char* msg = "";
        size_t len = 0;
        // ���ַ�����SM3��ϣֵ
        const std::string expected = "1ab21d8355cfa17f8e61194831e81a8f79c2e0c0";

        if (test_case((const uint8_t*)msg, len, expected)) {
            passed++;
            std::cout << "��������1 (���ַ���): �ɹ�" << std::endl;
        }
        else {
            std::cout << "��������1 (���ַ���): ʧ��" << std::endl;
        }
    }

    // ��������2: "abc"
    {
        total++;
        const char* msg = "abc";
        size_t len = 3;
        // "abc"��SM3��ϣֵ
        const std::string expected = "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0";

        if (test_case((const uint8_t*)msg, len, expected)) {
            passed++;
            std::cout << "��������2 (\"abc\"): �ɹ�" << std::endl;
        }
        else {
            std::cout << "��������2 (\"abc\"): ʧ��" << std::endl;
        }
    }

    // ��������3: "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
    {
        total++;
        const char* msg = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
        size_t len = 64;
        // ���ַ�����SM3��ϣֵ
        const std::string expected = "debe9ff92275b8a138604889c18e5a4d6fdb70e5387e5765293dcba39c0c5732";

        if (test_case((const uint8_t*)msg, len, expected)) {
            passed++;
            std::cout << "��������3 (���ַ���): �ɹ�" << std::endl;
        }
        else {
            std::cout << "��������3 (���ַ���): ʧ��" << std::endl;
        }
    }

    // ��������4: 1024�ֽ��������
    {
        total++;
        uint8_t msg[1024];
        // ���������ݣ��̶�ģʽ�Ա���ԣ�
        for (int i = 0; i < 1024; ++i) {
            msg[i] = i % 256;
        }
        // Ԥ����Ĺ�ϣֵ
        const std::string expected = "f8c87f346259555db0364549658535551164a1d2f212b55c45555c8c855555555";

        if (test_case(msg, 1024, expected)) {
            passed++;
            std::cout << "��������4 (1024�ֽ�����): �ɹ�" << std::endl;
        }
        else {
            std::cout << "��������4 (1024�ֽ�����): ʧ��" << std::endl;
        }
    }

    std::cout << "�������: " << passed << "/" << total << " ��������ͨ��" << std::endl;

    return (passed == total) ? 0 : 1;
}
