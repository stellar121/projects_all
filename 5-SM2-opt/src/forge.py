from gmssl import sm2, sm3, func
import hashlib
import base58

class SM2Satoshi:
    def __init__(self):
        self.private_key, self.public_key = self.generate_keypair()
        self.sm2_crypt = sm2.CryptSM2(public_key=self.public_key, private_key=self.private_key)
        self.fake_address = self.generate_fake_address()

    def generate_keypair(self):
        private_key = func.random_hex(64)
        sm2_ins = sm2.CryptSM2(private_key=private_key, public_key='')
        public_key = sm2_ins._kg(int(private_key, 16), sm2.default_ecc_table['g'])
        return private_key, public_key

    def sign(self, data: bytes) -> str:
        digest = sm3.sm3_hash(func.bytes_to_list(data))
        digest_bytes = bytes.fromhex(digest)
        random_k = func.random_hex(64)
        return self.sm2_crypt.sign(digest_bytes, random_k)

    def verify(self, data: bytes, signature: str) -> bool:
        digest = sm3.sm3_hash(func.bytes_to_list(data))
        digest_bytes = bytes.fromhex(digest)
        return self.sm2_crypt.verify(signature, digest_bytes)

    def generate_fake_address(self) -> str:
        # 1. 模拟公钥哈希（类似 Bitcoin: RIPEMD160(SHA256(pubkey)))
        pub_bytes = bytes.fromhex(self.public_key)
        sha256_digest = hashlib.sha256(pub_bytes).digest()
        ripemd160 = hashlib.new('ripemd160', sha256_digest).digest()

        # 2. 添加前缀 0x00 (P2PKH)
        versioned = b'\x00' + ripemd160

        # 3. 计算校验和（前 4 字节）
        checksum = hashlib.sha256(hashlib.sha256(versioned).digest()).digest()[:4]

        # 4. 拼接并编码为 base58check
        address_bytes = versioned + checksum
        base58_address = base58.b58encode(address_bytes).decode()
        return base58_address

if __name__ == "__main__":
    satoshi = SM2Satoshi()

    print("假冒地址（Base58Check）:", satoshi.fake_address)
    print("私钥:", satoshi.private_key)
    print("公钥:", satoshi.public_key)

    message = b'I am Nakamoto. Trust me!'
    signature = satoshi.sign(message)

    print("签名:", signature)

    # 验签（应成功）
    is_valid = satoshi.verify(message, signature)
    print("正确公钥验签结果:", is_valid)

    # 模拟错误验证：使用错误公钥（应失败）
    fake_pub = '04' + '1'*128
    sm2_invalid = sm2.CryptSM2(public_key=fake_pub, private_key='')
    digest = sm3.sm3_hash(func.bytes_to_list(message))
    digest_bytes = bytes.fromhex(digest)
    fake_result = sm2_invalid.verify(signature, digest_bytes)

    print("使用伪造公钥验签结果:", fake_result)
