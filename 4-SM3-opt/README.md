# SM3的软件实现与优化 

## 项目目录结构

```text
.
├── include/
│   ├── merkle_sm3.h         # Merkle 树构建与验证接口，基于 SM3 的哈希认证框架
│   └── sm3.h                # SM3 哈希函数声明，包括基础版、AVX2 和 AVX512 优化接口
├── src/
│   ├── sm3_utils.cpp        # SM3 中的辅助函数模块：位运算、常量定义、消息填充等基础功能
│   ├── sm3_basic.cpp        # 标准 C++ 实现的 SM3 哈希函数
│   ├── sm3_simd_avx2.cpp    # 使用 AVX2 SIMD 指令加速实现的 SM3
│   └── sm3_simd_avx512.cpp  # 使用 AVX512 指令集实现的高级并行优化版本
├── test/
│   ├── test_sm3.cpp         # 单元测试代码
│   ├── test_sm3_attack.cpp  # 长度扩展攻击文件
│   ├── test_sm3_merkle.cpp  # 根据RFC6962构建Merkle树
│   └── benchmark_sm3.cpp    # 用于比较不同实现的性能
├── project2.pdf             # 项目分析报告
└── README.md                # 仓库说明文档
```

## 具体的算法原理、代码分析、实验结果与分析见报告文档————project2.pdf
