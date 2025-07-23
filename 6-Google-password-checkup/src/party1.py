from crypto_utils import H, gen_exponent, modexp, ASum

class Party1:
    def __init__(self, v_list):
        # Party1 拥有的标识符集合 v_list，例如 ['u1', 'u2', 'u3']
        self.v_list = v_list
        self.k1 = gen_exponent()  # P1 随机选取私钥指数 k1
        self.intersection = []    # 用于记录交集元素在 P2 中的索引

    # 协议第一轮：对每个 v 进行 H(v)^k1 运算后发送给 P2
    def round1(self):
        return [modexp(H(v), self.k1) for v in self.v_list]

    # 协议第三轮：接收 P2 返回的 (H(w)^k2, AEnc(t)) 对，判断是否属于交集
    def round3(self, data_from_p2, pk):
        intersection = []     # 记录交集在 P2 数据中的索引
        encrypted_vals = []   # 存储交集中对应的加密值 AEnc(t)

        for (val, ct) in data_from_p2:
            val1 = modexp(val, self.k1)  # 还原为 H(w)^{k1*k2}
            if val1 in self.Z:           # 若出现在 P1 Round1 中的哈希集合中，则为交集
                j = self.Z.index(val1)
                intersection.append(j)
                encrypted_vals.append(ct)

        self.intersection = intersection  # 保存交集索引
        ct_sum = ASum(encrypted_vals)     # 对交集项的 AEnc(t) 同态求和
        return ct_sum

    # 设置来自 P2 的 Z 集合，即 H(w)^k2 的集合（Round2 中发来）
    def set_Z(self, Z):
        self.Z = Z
