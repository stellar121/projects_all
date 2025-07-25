# SM2 的软件实现优化

## 项目目录结构

```text
.
├── src/
│   ├── forge.py                  # 用于伪造签名的实现
│   ├── poc.py                    # 用于签名误用漏洞的 PoC 验证
│   ├── sm2.py                    # SM2 算法实现
│   ├── sm2_cryptor.py            # SM2 加解密与签名类
├── test/
│   ├── test_poc.py               # 验证签名误用漏洞的测试
│   ├── test_sign_verify.py       # SM2 签名验证相关测试
├── project5.pdf                  # 项目报告
├── README.md                     # 项目说明文件（本文件）
└── requirements.txt              # 依赖列表

## 具体的算法原理、代码分析、实验结果与分析见报告文档————project5.pdf
