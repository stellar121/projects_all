# Google Password Checkup PSI 协议实现（基于 DDH 的私有交集和）

本项目实现了来自论文 [Privacy Preserving Password Breach Detection using Bloom Filters](https://eprint.iacr.org/2019/723.pdf) 中提出的 **Google Password Checkup 验证协议**（如图所示），该协议基于 DDH 假设和加法同态加密，实现两方私有集合交集和（PSI-Sum）的高效计算。

本项目通过 Python 编写，使用 Paillier 加密系统实现同态加密运算，模拟 Google 在客户端和服务端之间部署的密码泄露检测协议。

---

## 一、协议简介

我实现的协议 \(\Pi_{\text{DDH}}\) 是一个三轮、半诚实安全的 PSI-Sum 协议，支持如下双方输入：

- **群设定**：一个满足 DDH 假设的素数阶群 \(\mathcal{G}\)
- **标识符空间**：\(\mathcal{U}\)，比如用户名或密码哈希
- **哈希函数**：\(H: \mathcal{U} \to \mathcal{G}\)，建模为随机预言机

### 二、双方输入

- **客户端 \(P_1\)**：拥有集合 \(V = \{v_i\}_{i=1}^{m_1}\)，其中 \(v_i \in \mathcal{U}\)
- **服务端 \(P_2\)**：拥有集合 \(W = \{(w_j, t_j)\}_{j=1}^{m_2}\)，其中 \(w_j \in \mathcal{U}, t_j \in \mathbb{Z}^+\)

---

## 三、协议流程概述

### 3.1 设置阶段
- 双方各自选择指数：
  - \(P_1\)：选择随机指数 \(k_1\)
  - \(P_2\)：选择随机指数 \(k_2\)，并生成 Paillier 加法同态加密密钥对 \((pk, sk)\)，将公钥 pk 发送给 \(P_1\)

---

### 第一轮（由 \(P_1\) 发起）

- 对每个 \(v_i \in V\)，计算：
  \[
  X_i = H(v_i)^{k_1}
  \]
- 将乱序后的 \(\{X_i\}\) 发给 \(P_2\)

---

### 第二轮（由 \(P_2\) 发起）

- 对收到的每个 \(X_i\)，再进行一次指数运算：
  \[
  Z_i = (H(v_i)^{k_1})^{k_2} = H(v_i)^{k_1 k_2}
  \]
  得到集合 \(Z = \{Z_i\}\)，发送给 \(P_1\)

- 对本地每个 \(w_j\)，计算：
  \[
  Y_j = H(w_j)^{k_2},\quad C_j = \text{AEnc}(t_j)
  \]
  得到加密对 \((Y_j, C_j)\)，乱序发送给 \(P_1\)

---

### 第三轮（由 \(P_1\) 完成匹配并求和）

- 对每个收到的 \(Y_j\)，计算：
  \[
  Y_j' = (H(w_j)^{k_2})^{k_1} = H(w_j)^{k_1 k_2}
  \]
- 若 \(Y_j' \in Z\)，则认为该元素出现在交集中，并收集对应密文 \(C_j\)

- 使用同态加密进行求和：
  \[
  \text{AEnc}(S_J) = \text{ASum}(\{C_j\}_{j \in J}) = \text{AEnc}\left(\sum_{j \in J} t_j\right)
  \]
- 随机化密文后，发送给 \(P_2\)

---

### 输出（由 \(P_2\) 解密）

- 使用私钥解密得到交集和：
  \[
  S_J = \sum_{j \in J} t_j
  \]

---

## 四、项目结构

├── crypto_utils.py # 哈希、群运算、Paillier 同态加密等函数
├── party1.py # 客户端 P1 协议逻辑（Round 1 & 3）
├── party2.py # 服务端 P2 协议逻辑（Round 2 & 解密）
├── protocol.py # 封装完整三轮协议执行流程
├── test_protocol.py # 示例测试入口
├── requirements.txt # Python 依赖（仅含 phe）
└── README.md # 项目说明文档


### 五、环境依赖

```bash
pip install -r requirements.txt

### 六、运行测试协议

python test_protocol.py

### 七、输出示例


Intersection indexes: [1, 2]
Intersection sum: 30

数据解释

P1 输入集合：['u1', 'u2', 'u3']

P2 输入集合：[('u2', 10), ('u3', 20), ('u4', 40)]

交集为：{'u2', 'u3'}

求和值为：10 + 20 = 30

### 八、参考论文

Henry Corrigan-Gibbs, Dan Boneh.Privacy Preserving Password Breach Detection using Bloom Filters, 2019.


