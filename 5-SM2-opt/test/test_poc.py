import unittest
from poc import correct_verify, wrong_verify_p_mod
from gmssl import sm3, func

class TestSM2PoC(unittest.TestCase):
    def setUp(self):
        self.fake_r = 1
        self.fake_s = 1
        self.message = b"Fake Signature PoC Test"
        self.e = sm3.sm3_hash(func.bytes_to_list(self.message))
        self.p_hex = "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"

    def test_correct_verification_should_fail(self):
        self.assertFalse(correct_verify(self.fake_r, self.fake_s, self.e))

    def test_wrong_verification_should_pass(self):
        self.assertTrue(wrong_verify_p_mod(self.fake_r, self.fake_s, self.e, self.p_hex))

if __name__ == "__main__":
    unittest.main()
