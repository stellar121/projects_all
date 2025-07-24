from gmssl import sm2, sm3, func

class SM2Cryptor:
    def __init__(self, private_key: str = None, public_key: str = None):
        """
        初始化 SM2 实例
        """
        if private_key and public_key:
            self.private_key = private_key
            self.public_key = public_key
        else:
            self.private_key, self.public_key = self.generate_keypair()
        self.sm2_crypt = sm2.CryptSM2(
            public_key=self.public_key, private_key=self.private_key
        )

    def generate_keypair(self):
        """
        随机生成 SM2 密钥对
        """
        private_key = func.random_hex(64)
        sm2_instance = sm2.CryptSM2(public_key='', private_key=private_key)
        public_key = sm2_instance._kg(int(private_key, 16), sm2.default_ecc_table['g'])
        return private_key, public_key

    def encrypt(self, plaintext: bytes) -> bytes:
        """
        加密
        """
        return self.sm2_crypt.encrypt(plaintext)

    def decrypt(self, ciphertext: bytes) -> bytes:
        """
        解密
        """
        return self.sm2_crypt.decrypt(ciphertext)

    def sign(self, data: bytes) -> str:
        """
        对数据签名（返回十六进制字符串）
        """
        digest = sm3.sm3_hash(func.bytes_to_list(data))
        digest_bytes = bytes.fromhex(digest)
        random_k = func.random_hex(64)
        return self.sm2_crypt.sign(digest_bytes, random_k)

    def verify(self, data: bytes, signature: str) -> bool:
        """
        验证签名，返回 True / False
        """
        digest = sm3.sm3_hash(func.bytes_to_list(data))
        digest_bytes = bytes.fromhex(digest)
        return self.sm2_crypt.verify(signature, digest_bytes)
