import unittest
from sm2_cryptor import SM2Cryptor

class TestSM2SignVerify(unittest.TestCase):
    def setUp(self):
        self.sm2_tool = SM2Cryptor()
        self.message = b"unit test message"

    def test_sign_and_verify(self):
        signature = self.sm2_tool.sign(self.message)
        result = self.sm2_tool.verify(self.message, signature)
        self.assertTrue(result)

    def test_verify_invalid_signature(self):
        signature = self.sm2_tool.sign(self.message)
        tampered_message = b"tampered message"
        result = self.sm2_tool.verify(tampered_message, signature)
        self.assertFalse(result)

if __name__ == "__main__":
    unittest.main()
