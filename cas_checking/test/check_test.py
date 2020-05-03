import unittest
from cas import CAS
from check import check


class MyTestCase(unittest.TestCase):
    def test_euler_cycle(self):
        self.assertTrue(
            check(
                [CAS(1, 2, True), CAS(2, 3, True), CAS(3, 1, True)],
                1
            )
        )

    def test_euler_path(self):
        self.assertTrue(
            check(
                [CAS(1, 2, True), CAS(2, 4, True), CAS(4, 3, True),
                 CAS(3, 2, True), CAS(2, 1, True), CAS(1, 2, True)],
                1
            )
        )

    def test_euler_75path_begin_in_not_init(self):
        self.assertFalse(
            check(
                [CAS(1, 2, True), CAS(2, 4, True), CAS(4, 3, True),
                 CAS(3, 2, True), CAS(2, 1, True), CAS(1, 2, True)],
                2
            )
        )


if __name__ == '__main__':
    unittest.main()
