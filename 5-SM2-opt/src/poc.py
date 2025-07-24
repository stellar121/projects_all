from gmssl import sm2, sm3, func
from hashlib import sha256

# 正常生成一对密钥
private_key = func.random_hex(64)
sm2_crypt = sm2.CryptSM2(private_key=private_key, public_key="")

# 模拟：使用假的签名(r, s)
fake_r = 1
fake_s = 1

# 模拟消息和摘要
msg = b'Fake Signature PoC Test'
e = sm3.sm3_hash(func.bytes_to_list(msg))

fake_pubkey = '04' + '1'*128
sm2_verify = sm2.CryptSM2(public_key=fake_pubkey, private_key='')

# 正确的验签逻辑
def correct_verify(r, s, e):
    # 验证：(e + x1) mod n == r
    # 我们模拟强行使 x1 + e mod p == r
    # 验证算法使用了 mod **n**
    return sm2_verify.verify(f'{r:064x}{s:064x}', bytes.fromhex(e))

# 错误的验签逻辑（误用了 mod p）
def wrong_verify_p_mod(r, s, e, p_hex):
    p = int(p_hex, 16)
    x1_fake = (int(r) - int(e, 16)) % p
    r_fake = (int(e, 16) + x1_fake) % p
    return r_fake == r

# 输出
print("正确验签是否通过？", correct_verify(fake_r, fake_s, e))
print("错误模数下是否通过？", wrong_verify_p_mod(fake_r, fake_s, e,
       "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"))