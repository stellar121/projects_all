import hashlib
import random
from phe import paillier

# 设定群 G：使用素数阶 p 的乘法群 Z_p^* 来模拟 DDH 安全群
PRIME_P = 2 ** 127 - 1  # 取一个大的素数，便于模运算

# 哈希函数 H：将输入字符串映射为群 G 中的元素
def H(x):
    h = hashlib.sha256()            # 使用 SHA-256 哈希函数
    h.update(x.encode('utf-8'))     # 编码字符串
    return int(h.hexdigest(), 16) % PRIME_P  # 将结果映射到模 p 的整数上

# 生成随机指数 k，用于 exponentiation
def gen_exponent():
    return random.randint(2, PRIME_P - 2)

# 群上指数运算：计算 base^exponent mod p
def modexp(base, exponent):
    return pow(base, exponent, PRIME_P)

# 生成 Paillier 同态加密的公私钥对
def paillier_keygen():
    public_key, private_key = paillier.generate_paillier_keypair()
    return public_key, private_key

# 同态加密：用公钥加密一个整数值 val
def AEnc(pk, val):
    return pk.encrypt(val)

# 同态解密：用私钥解密密文 ciphertext
def ADec(sk, ciphertext):
    return sk.decrypt(ciphertext)

# 同态求和：将密文列表相加，利用 Paillier 的加法同态性
def ASum(ciphertexts):
    total = ciphertexts[0]
    for ct in ciphertexts[1:]:
        total += ct
    return total

# 同态密文刷新（re-randomize）：用于打乱密文，防止关联性攻击
def ARefresh(ciphertext):
    return ciphertext.public_key.encrypt(ciphertext.ciphertext(False))
