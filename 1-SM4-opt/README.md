# SM4的软件实现与优化 

## 项目目录结构

```text
.
├── include/
│   ├── sm4.h                 # SM4 基础算法功能声明
│   ├── sm4_gcm.h             # SM4 - GCM 基础工作模式功能声明
│   └── sm4_gcm_optimized.h   # SM4 - GCM 优化版本功能声明
├── src/
│   ├── sm4_aesni.cpp         # 基于 AESNI 指令集对 SM4 进行优化的实现代码，利用 AESNI 加速相关运算
│   ├── sm4_basic.cpp         # SM4 算法的基础实现代码，包含轮函数、S 盒、线性变换等核心逻辑
│   ├── sm4_gcm.cpp           # SM4 - GCM 基础工作模式的实现代码，实现 CTR 加密、伽罗瓦域认证等流程
│   ├── sm4_gcm_optimized.cpp # SM4 - GCM 优化版本的实现代码，集成各类优化手段提升性能
│   ├── sm4_gfni.cpp          # 基于 GFNI 指令集对 SM4 进行优化的实现代码，借助 GFNI 加速 SM4 运算
│   └── sm4_ttable.cpp        # 基于 T - table 技术对 SM4 进行优化的实现代码，通过预计算表提升效率
├── test/
│   ├── test_sm4.cpp          # SM4 基础算法功能测试代码，验证 sm4.h 及 sm4_basic.cpp 等的正确性
│   ├── test_sm4_gcm.cpp      # SM4 - GCM 基础工作模式测试代码，测试 sm4_gcm.h 和 sm4_gcm.cpp 的功能
│   └── test_sm4_gcm_optimized.cpp # SM4 - GCM 优化版本测试代码，验证优化后的功能及性能表现
├── project1.pdf             # 项目分析报告
└── README.md                # 仓库说明文档
```

## 具体的算法原理、代码分析、实验结果与分析见报告文档————project1.pdf
