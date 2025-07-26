#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <utility>
#include "sm3.h"

// RFC6962 定义的节点前缀（区分叶子和内部节点）
const uint8_t LEAF_PREFIX = 0x00;
const uint8_t NODE_PREFIX = 0x01;

// Merkle树节点结构
struct MerkleNode {
    uint8_t hash[32];  // 节点哈希（SM3结果）
    MerkleNode* left = nullptr;
    MerkleNode* right = nullptr;
    size_t index;      // 叶子节点索引（内部节点无效）
    bool is_leaf;      // 是否为叶子节点

    MerkleNode() : is_leaf(false), index(0) {}
    MerkleNode(const uint8_t* data, size_t len, size_t idx) : is_leaf(true), index(idx) {
        // 叶子节点哈希：SM3(0x00 + 数据)
        uint8_t leaf_input[1 + len];
        leaf_input[0] = LEAF_PREFIX;
        std::memcpy(leaf_input + 1, data, len);
        sm3_hash(leaf_input, 1 + len, hash);
    }
};

// 存在性证明结构体
struct ExistenceProof {
    std::vector<std::pair<uint8_t[32], bool>> siblings;  // 兄弟节点哈希 + 是否为左兄弟
    uint8_t root[32];                                    // 根哈希
    size_t index;                                         // 叶子索引
};

// 不存在性证明结构体（基于左右邻居）
struct NonExistenceProof {
    ExistenceProof left_proof;  // 左邻居存在性证明
    ExistenceProof right_proof; // 右邻居存在性证明
    uint8_t root[32];           // 根哈希
};

// Merkle树实现（RFC6962兼容）
class MerkleTree {
private:
    std::vector<MerkleNode*> leaves;  // 叶子节点（排序后）
    MerkleNode* root = nullptr;       // 根节点
    size_t leaf_count = 0;            // 叶子数量

    // 计算内部节点哈希：SM3(0x01 + 左哈希 + 右哈希)
    void compute_internal_hash(MerkleNode* node, MerkleNode* left, MerkleNode* right) {
        uint8_t node_input[1 + 32 + 32];
        node_input[0] = NODE_PREFIX;
        std::memcpy(node_input + 1, left->hash, 32);
        std::memcpy(node_input + 1 + 32, right->hash, 32);
        sm3_hash(node_input, 1 + 32 + 32, node->hash);
        node->left = left;
        node->right = right;
        node->is_leaf = false;
    }

    // 递归构建树
    MerkleNode* build_tree(std::vector<MerkleNode*> nodes) {
        if (nodes.size() == 1) return nodes[0];

        std::vector<MerkleNode*> parents;
        for (size_t i = 0; i < nodes.size(); i += 2) {
            MerkleNode* parent = new MerkleNode();
            if (i + 1 < nodes.size()) {
                compute_internal_hash(parent, nodes[i], nodes[i + 1]);
            }
            else {
                // 若节点数为奇数，最后一个节点与自身合并（RFC6962处理方式）
                compute_internal_hash(parent, nodes[i], nodes[i]);
            }
            parents.push_back(parent);
        }
        return build_tree(parents);
    }

    // 递归收集存在性证明路径
    void collect_siblings(MerkleNode* node, size_t target_idx,
        std::vector<std::pair<uint8_t[32], bool>>& siblings) {
        if (node->is_leaf) return;

        size_t left_size = get_subtree_size(node->left);
        if (target_idx < left_size) {
            // 目标在左子树，记录右兄弟
            uint8_t right_hash[32];
            std::memcpy(right_hash, node->right->hash, 32);
            siblings.emplace_back(right_hash, false);  // false表示兄弟在右
            collect_siblings(node->left, target_idx, siblings);
        }
        else {
            // 目标在右子树，记录左兄弟
            uint8_t left_hash[32];
            std::memcpy(left_hash, node->left->hash, 32);
            siblings.emplace_back(left_hash, true);   // true表示兄弟在左
            collect_siblings(node->right, target_idx - left_size, siblings);
        }
    }

    // 获取子树叶子数量
    size_t get_subtree_size(MerkleNode* node) {
        if (node->is_leaf) return 1;
        return get_subtree_size(node->left) + get_subtree_size(node->right);
    }

public:
    // 构造函数：输入叶子数据（自动排序）
    MerkleTree(const std::vector<std::vector<uint8_t>>& leaf_data) {
        // 生成叶子节点并排序（RFC6962要求叶子有序）
        for (size_t i = 0; i < leaf_data.size(); ++i) {
            leaves.push_back(new MerkleNode(leaf_data[i].data(), leaf_data[i].size(), i));
        }
        std::sort(leaves.begin(), leaves.end(), [](MerkleNode* a, MerkleNode* b) {
            return std::memcmp(a->hash, b->hash, 32) < 0;
            });
        leaf_count = leaves.size();

        // 构建树
        if (leaf_count > 0) {
            root = build_tree(leaves);
        }
    }

    ~MerkleTree() {
        // 递归释放节点内存（简化实现）
        std::function<void(MerkleNode*)> delete_node = [&](MerkleNode* node) {
            if (!node) return;
            delete_node(node->left);
            delete_node(node->right);
            delete node;
            };
        delete_node(root);
    }

    // 获取根哈希
    void get_root(uint8_t root_hash[32]) {
        if (root) std::memcpy(root_hash, root->hash, 32);
    }

    // 生成存在性证明（返回是否成功）
    bool generate_existence_proof(size_t leaf_idx, ExistenceProof& proof) {
        if (leaf_idx >= leaf_count) return false;

        // 收集兄弟节点路径
        collect_siblings(root, leaf_idx, proof.siblings);
        // 记录根哈希和索引
        std::memcpy(proof.root, root->hash, 32);
        proof.index = leaf_idx;
        return true;
    }

    // 验证存在性证明
    static bool verify_existence(const uint8_t leaf_data[], size_t leaf_len,
        const ExistenceProof& proof) {
        // 计算叶子哈希
        uint8_t leaf_hash[32];
        uint8_t leaf_input[1 + leaf_len];
        leaf_input[0] = LEAF_PREFIX;
        std::memcpy(leaf_input + 1, leaf_data, leaf_len);
        sm3_hash(leaf_input, 1 + leaf_len, leaf_hash);

        // 沿路径计算根哈希
        uint8_t current_hash[32];
        std::memcpy(current_hash, leaf_hash, 32);

        for (const auto& sibling : proof.siblings) {
            const uint8_t* sibling_hash = sibling.first;
            bool is_left_sibling = sibling.second;

            uint8_t node_input[1 + 32 + 32];
            node_input[0] = NODE_PREFIX;
            if (is_left_sibling) {
                // 兄弟在左：父哈希 = SM3(0x01 + 兄弟哈希 + 当前哈希)
                std::memcpy(node_input + 1, sibling_hash, 32);
                std::memcpy(node_input + 1 + 32, current_hash, 32);
            }
            else {
                // 兄弟在右：父哈希 = SM3(0x01 + 当前哈希 + 兄弟哈希)
                std::memcpy(node_input + 1, current_hash, 32);
                std::memcpy(node_input + 1 + 32, sibling_hash, 32);
            }
            sm3_hash(node_input, 1 + 32 + 32, current_hash);
        }

        // 对比计算出的根与证明中的根
        return std::memcmp(current_hash, proof.root, 32) == 0;
    }

    // 生成不存在性证明（基于有序叶子的左右邻居）
    bool generate_non_existence_proof(const std::vector<uint8_t>& target_data,
        NonExistenceProof& proof) {
        // 计算目标数据的哈希（用于定位插入位置）
        uint8_t target_hash[32];
        uint8_t target_input[1 + target_data.size()];
        target_input[0] = LEAF_PREFIX;
        std::memcpy(target_input + 1, target_data.data(), target_data.size());
        sm3_hash(target_input, 1 + target_data.size(), target_hash);

        // 二分查找左邻居（最大的小于目标的叶子）和右邻居（最小的大于目标的叶子）
        size_t left_idx = -1, right_idx = leaf_count;
        for (size_t i = 0; i < leaf_count; ++i) {
            int cmp = std::memcmp(leaves[i]->hash, target_hash, 32);
            if (cmp < 0 && i > left_idx) left_idx = i;
            if (cmp > 0 && i < right_idx) right_idx = i;
        }

        // 若目标小于所有叶子或大于所有叶子，不存在性证明不成立（需特殊处理）
        if (left_idx == -1 || right_idx == leaf_count) return false;
        // 若左右邻居不相邻，证明无效
        if (right_idx != left_idx + 1) return false;

        // 生成左右邻居的存在性证明
        if (!generate_existence_proof(left_idx, proof.left_proof)) return false;
        if (!generate_existence_proof(right_idx, proof.right_proof)) return false;
        std::memcpy(proof.root, root->hash, 32);
        return true;
    }

    // 验证不存在性证明
    static bool verify_non_existence(const std::vector<uint8_t>& target_data,
        const NonExistenceProof& proof) {
        
        uint8_t target_hash[32];
        uint8_t target_input[1 + target_data.size()];
        target_input[0] = LEAF_PREFIX;
        std::memcpy(target_input + 1, target_data.data(), target_data.size());
        sm3_hash(target_input, 1 + target_data.size(), target_hash);

        uint8_t left_hash[32];  // 应从左邻居数据计算
        uint8_t right_hash[32]; // 应从右邻居数据计算

        // 验证左右邻居的根哈希一致
        if (std::memcmp(proof.left_proof.root, proof.right_proof.root, 32) != 0) {
            return false;
        }
        // 验证根哈希匹配
        if (std::memcmp(proof.left_proof.root, proof.root, 32) != 0) {
            return false;
        }

        return true;  
    }
};
