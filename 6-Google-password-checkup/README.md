# Google Password Checkup PSI 协议实现（基于 DDH 的私有交集和）

## 项目结构
.
├── src/
│   ├── crypto_utils.py           # 哈希、群运算、Paillier 同态加密等函数
│   ├── party1.py                 # 客户端 P1 协议逻辑（Round 1 & 3）
│   ├── party2.py                 # 服务端 P2 协议逻辑（Round 2 & 解密）
│   ├── protocol.py               # 封装完整三轮协议执行流程
├── test/
│   └── test_protocol.py          # 示例测试入口
├── project6.pdf                  # 项目说明文件（本文件）
├── README.md                     # 项目说明文件（本文件）
└── requirements.txt              # 依赖列表



