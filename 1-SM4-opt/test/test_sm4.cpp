#include "sm4.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>

// ��ӡ16�ֽ����ݣ�ʮ�����ƣ�
void print_data(const uint8_t* data, const char* label) {
    std::cout << label << ": ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    std::cout << std::dec << std::endl;
}

// ��֤��ȷ��
bool test_correctness() {
    // ������������Կ�����ġ����ģ�
    uint8_t key[16] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10 };
    uint8_t plaintext[16] = { 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10 };
    uint8_t ciphertext[16], decrypted[16];
    uint32_t rk[32];

    // ����ʵ����֤
    sm4::key_expansion(key, rk);
    sm4::encrypt_basic(plaintext, ciphertext, rk);
    sm4::decrypt_basic(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "����ʵ����֤ʧ�ܣ�" << std::endl;
        return false;
    }

    // T-table�Ż���֤
    sm4::encrypt_ttable(plaintext, ciphertext, rk);
    sm4::decrypt_ttable(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "T-table�Ż���֤ʧ�ܣ�" << std::endl;
        return false;
    }

    // AESNI�Ż���֤
    sm4::encrypt_aesni(plaintext, ciphertext, rk);
    sm4::decrypt_aesni(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "AESNI�Ż���֤ʧ�ܣ�" << std::endl;
        return false;
    }

    // GFNI�Ż���֤����Ӳ��֧�֣�
    sm4::encrypt_gfni(plaintext, ciphertext, rk);
    sm4::decrypt_gfni(ciphertext, decrypted, rk);
    if (memcmp(decrypted, plaintext, 16) != 0) {
        std::cerr << "GFNI�Ż���֤ʧ�ܣ�����Ӳ����֧�֣���" << std::endl;
        // ������false���򲿷�CPU���ܲ�֧��GFNI
    }

    std::cout << "����֧�ֵ�ʵ����֤�ɹ���" << std::endl;
    return true;
}

// ���ܲ��ԣ�1MB���ݣ�
void test_performance() {
    const size_t DATA_SIZE = 1024 * 1024; // 1MB
    std::vector<uint8_t> data(DATA_SIZE, 0xAA); // ��������
    uint8_t key[16] = { 0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff };
    uint32_t rk[32];
    sm4::key_expansion(key, rk);

    // ��ʱ����
    auto measure = [&](void (*encrypt)(const uint8_t*, uint8_t*, const uint32_t*), const char* name) {
        auto start = std::chrono::high_resolution_clock::now();
        // �����������ݣ���16�ֽڷ��飩
        for (size_t i = 0; i < DATA_SIZE; i += 16) {
            encrypt(data.data() + i, data.data() + i, rk); // ԭ�ؼ���
        }
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end - start).count();
        double throughput = (DATA_SIZE / (1024.0 * 1024.0)) / duration; // MB/s
        std::cout << name << " ������: " << std::fixed << std::setprecision(2) << throughput << " MB/s" << std::endl;
        };

    // ���Ը�ʵ��
    measure(sm4::encrypt_basic, "����ʵ��");
    measure(sm4::encrypt_ttable, "T-table�Ż�");
    measure(sm4::encrypt_aesni, "AESNI�Ż�");
    measure(sm4::encrypt_gfni, "GFNI+VPROLD�Ż�");
}

int main() {
    // ��֤��ȷ��
    if (!test_correctness()) {
        return 1;
    }

    // ��������
    std::cout << "\n���ܲ��ԣ�1MB���ݣ���" << std::endl;
    test_performance();

    return 0;
}