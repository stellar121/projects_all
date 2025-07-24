# SM2 的软件实现优化

## 一、SM2 加解密过程（基于椭圆曲线）

### 加密流程（发送方）：

1. 生成随机数 $k \in [1, n-1]$
2. 计算椭圆曲线点：$C_1 = kG$
3. 计算共享密钥点：$S = kP_B = (x_2, y_2)$
4. 使用 $KDF(x_2 \parallel y_2, k_{\text{len}})$ 生成密钥 $t$
5. 计算：$C_2 = M \oplus t$ （$M$ 为明文）
6. 计算哈希：$C_3 = Hash(x_2 \parallel M \parallel y_2)$
7. 输出密文：$C = C_1 \parallel C_3 \parallel C_2$

### 解密流程（接收方）：

1. 从密文中解析出 $C_1, C_2, C_3$
2. 验证 $C_1$ 是否为合法曲线点
3. 计算共享点：$S = d_B \cdot C_1 = (x_2, y_2)$
4. 使用 $KDF(x_2 \parallel y_2, k_{\text{len}})$ 得到密钥 $t$
5. 恢复明文：$M = C_2 \oplus t$
6. 验证：$C_3 \stackrel{?}{=} Hash(x_2 \parallel M \parallel y_2)$

---

## 二、SM2 签名与验证过程

### 签名流程：

1. 生成随机数 $k \in [1, n-1]$
2. 计算点：$P_1 = kG = (x_1, y_1)$
3. 计算摘要：$e = Hash(Z_A \parallel M)$，其中 $Z_A$ 为标识哈希
4. 计算：
   $$
   r = (e + x_1) \bmod n
   $$
   若 $r = 0$ 或 $r + k = n$，则重签
5. 计算：
   $$
   s = \left(1 + d_A\right)^{-1} \cdot \left(k - r \cdot d_A\right) \bmod n
   $$
   若 $s = 0$，重签
6. 输出签名对：$(r, s)$

### 验证过程：

1. 验证 $r, s \in [1, n-1]$，否则拒绝
2. 计算摘要：$e = Hash(Z_A \parallel M)$
3. 计算：
   $$
   t = (r + s) \bmod n
   $$
   若 $t = 0$，拒绝
4. 计算点：
   $$
   (x_1', y_1') = sG + tP_A
   $$
5. 计算：
   $$
   R = (e + x_1') \bmod n
   $$
6. 验证：
   $$
   R \stackrel{?}{=} r
   $$

---

## 三、补充说明

- 所有运算均在椭圆曲线定义的有限域 $\mathbb{F}_p$ 和群阶 $n$ 下进行。
- $Z_A$ 为用户标识、公钥、系统参数等计算得到的预处理哈希值，用于绑定签名者身份。
- $KDF$ 是密钥派生函数，常基于 $SM3$ 实现，用于生成加解密所用对称密钥流。
