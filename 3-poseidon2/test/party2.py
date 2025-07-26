from crypto_utils import H, gen_exponent, modexp, paillier_keygen, AEnc

class Party2:
    def __init__(self, w_t_list):
        # Party2 拥有的标识符-值对 (w, t)，例如 [('u2', 10), ('u3', 20)]
        self.w_t_list = w_t_list
        self.k2 = gen_exponent()            # P2 随机生成指数 k2
        self.pk, self.sk = paillier_keygen()  # 生成 Paillier 加密的公钥和私钥

    # 获取 Paillier 公钥，发送给 Party1 用于同态加密
    def get_public_key(self):
        return self.pk

    # 协议第二轮：
    # 接收来自 P1 的 H(v)^k1 值，对其做幂运算（^k2），得到 H(v)^{k1*k2} 作为 Z
    # 同时将本地的 H(w)^k2 和 AEnc(t) 对组成列表返回
    def round2(self, data_from_p1):
        # 对每个 H(v)^k1 再做一次 ^k2，得到 H(v)^{k1*k2}
        self.Z = [modexp(x, self.k2) for x in data_from_p1]

        results = []
        for (w, t) in self.w_t_list:
            val = modexp(H(w), self.k2)   # 计算 H(w)^k2
            ct = AEnc(self.pk, t)        # 加密 value t，得到 AEnc(t)
            results.append((val, ct))    # 返回 (H(w)^k2, AEnc(t)) 对

        return self.Z, results

    # 解密 P1 返回的 Paillier 密文之和（即交集中所有 t_j 的总和）
    def decrypt_sum(self, ct_sum):
        return self.sk.decrypt(ct_sum)
