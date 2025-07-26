#include "merkle_sm3.h"
#include <iostream>
#include <iomanip>
#include <random>

// 生成随机叶子数据
std::vector<std::vector<uint8_t>> generate_random_leaves(size_t count, size_t data_size = 32) {
    std::vector<std::vector<uint8_t>> leaves;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);

    for (size_t i = 0; i < count; ++i) {
        std::vector<uint8_t> data(data_size);
        for (auto& b : data) b = dist(gen);
        leaves.push_back(data);
    }
    return leaves;
}

// 打印哈希值
void print_hash(const uint8_t hash[32], const std::string& label) {
    std::cout << label << ": ";
    for (int i = 0; i < 32; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    std::cout << std::dec << std::endl;
}

int main() {
    // 1. 生成10万个叶子节点
    const size_t LEAF_COUNT = 100000;
    std::cout << "生成" << LEAF_COUNT << "个随机叶子节点..." << std::endl;
    auto leaves = generate_random_leaves(LEAF_COUNT);

    // 2. 构建Merkle树
    std::cout << "构建Merkle树..." << std::endl;
    MerkleTree merkle_tree(leaves);
    uint8_t root_hash[32];
    merkle_tree.get_root(root_hash);
    print_hash(root_hash, "Merkle树根哈希");

    // 3. 测试存在性证明
    size_t test_idx = 12345;  // 测试第12345个叶子
    ExistenceProof exist_proof;
    if (merkle_tree.generate_existence_proof(test_idx, exist_proof)) {
        std::cout << "\n存在性证明测试（索引=" << test_idx << "）: ";
        bool valid = MerkleTree::verify_existence(leaves[test_idx].data(), leaves[test_idx].size(), exist_proof);
        std::cout << (valid ? "验证成功" : "验证失败") << std::endl;
    }

    // 4. 测试不存在性证明（使用一个不在叶子中的数据）
    std::vector<uint8_t> non_exist_data(32, 0xAA);  // 构造一个不存在的叶子
    NonExistenceProof non_exist_proof;
    if (merkle_tree.generate_non_existence_proof(non_exist_data, non_exist_proof)) {
        std::cout << "不存在性证明测试: ";
        bool valid = MerkleTree::verify_non_existence(non_exist_data, non_exist_proof);
        std::cout << (valid ? "验证成功" : "验证失败") << std::endl;
    }
    else {
        std::cout << "不存在性证明生成失败（可能目标在叶子范围外）" << std::endl;
    }

    return 0;
}
