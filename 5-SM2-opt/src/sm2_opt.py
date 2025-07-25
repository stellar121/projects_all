from gmssl import sm2, sm3, func
import hashlib
import time
from concurrent.futures import ThreadPoolExecutor

class SM2Cryptor:
    def __init__(self, private_key: str = None, public_key: str = None, use_sha256: bool = False):
        """
        初始化 SM2 实例
        """
        self.use_sha256 = use_sha256  # 使用更高效的 sha256
        if private_key and public_key:
            self.private_key = private_key
            self.public_key = public_key
        else:
            self.private_key, self.public_key = self.generate_keypair()
        self.sm2_crypt = sm2.CryptSM2(public_key=self.public_key, private_key=self.private_key)
        self.digest_cache = {}  # 用于缓存已计算的消息摘要

    def generate_keypair(self):
        """
        随机生成 SM2 密钥对
        """
        private_key = func.random_hex(64)
        sm2_instance = sm2.CryptSM2(public_key='', private_key=private_key)
        public_key = sm2_instance._kg(int(private_key, 16), sm2.default_ecc_table['g'])
        return private_key, public_key

    def get_digest(self, data: bytes) -> bytes:
        """
        获取消息摘要，使用缓存避免重复计算
        """
        if data in self.digest_cache:
            return self.digest_cache[data]
        if self.use_sha256:
            digest = hashlib.sha256(data).hexdigest()  # 使用更高效的 SHA256
        else:
            digest = sm3.sm3_hash(func.bytes_to_list(data))  # 使用国密 SM3
        digest_bytes = bytes.fromhex(digest)
        self.digest_cache[data] = digest_bytes  # 缓存结果
        return digest_bytes

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
        digest_bytes = self.get_digest(data)  # 使用缓存获取消息摘要
        random_k = func.random_hex(64)
        return self.sm2_crypt.sign(digest_bytes, random_k)

    def verify(self, data: bytes, signature: str) -> bool:
        """
        验证签名，返回 True / False
        """
        digest_bytes = self.get_digest(data)  # 使用缓存获取消息摘要
        return self.sm2_crypt.verify(signature, digest_bytes)

    def parallel_sign(self, data_list):
        """
        并行签名处理多个数据项
        """
        with ThreadPoolExecutor() as executor:
            results = executor.map(self.sign, data_list)
        return list(results)

    def generate_fake_address(self) -> str:
        """
        模拟中本聪地址生成
        """
        pub_bytes = bytes.fromhex(self.public_key)
        sha256_digest = hashlib.sha256(pub_bytes).digest()
        ripemd160 = hashlib.new('ripemd160', sha256_digest).digest()

        versioned = b'\x00' + ripemd160
        checksum = hashlib.sha256(hashlib.sha256(versioned).digest()).digest()[:4]
        address_bytes = versioned + checksum
        base58_address = func.base58encode(address_bytes).decode()
        return base58_address


# 运行时间性能测试
def time_test(sm2_tool):
    message = b"Hello, SM2!"

    # 测试签名和验签时间
    start_time = time.time()
    signature = sm2_tool.sign(message)
    is_valid = sm2_tool.verify(message, signature)
    sign_verify_time = time.time() - start_time

    # 测试加解密时间
    start_time = time.time()
    ciphertext = sm2_tool.encrypt(message)
    decrypted = sm2_tool.decrypt(ciphertext)
    encryption_decryption_time = time.time() - start_time

    # 测试并行签名时间
    data_list = [b"message 1", b"message 2", b"message 3", b"message 4"]
    start_time_parallel = time.time()
    parallel_signatures = sm2_tool.parallel_sign(data_list)
    parallel_sign_time = time.time() - start_time_parallel

    # 输出测试结果
    print(f"签名和验签时间: {sign_verify_time:.6f}秒")
    print(f"加解密时间: {encryption_decryption_time:.6f}秒")
    print(f"并行签名时间: {parallel_sign_time:.6f}秒")

    return {
        "sign_verify_time": sign_verify_time,
        "encryption_decryption_time": encryption_decryption_time,
        "parallel_sign_time": parallel_sign_time
    }

# 创建实例并运行时间测试
sm2_tool = SM2Cryptor(use_sha256=True)
time_test_results = time_test(sm2_tool)
