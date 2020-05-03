import unittest

from cas import CAS
from check_restricted import check


class CheckRestrictedTestCase(unittest.TestCase):
    def test_sequential(self):
        self.assertTrue(
            check(
                [
                    [CAS(3, 5, False), CAS(1, 2, True), CAS(2, 7, True)]
                ],
                1
            )
        )

    def test_single_no_actions(self):
        self.assertTrue(
            check(
                [
                    [CAS(3, 5, False)]
                ],
                1
            )
        )

    def test_sequential_no_actions(self):
        self.assertTrue(
            check(
                [
                    [CAS(3, 5, False), CAS(2, 4, False)]
                ],
                1
            )
        )

    def test_single_incorrect(self):
        self.assertFalse(
            check(
                [
                    [CAS(1, 5, False)]
                ],
                1
            )
        )

    def test_parallel(self):
        self.assertTrue(
            check(
                [
                    [CAS(3, 5, False), CAS(1, 2, True), CAS(7, 9, True)],
                    [CAS(2, 7, True), CAS(7, 8, False), CAS(9, 10, True)],
                ],
                1
            )
        )

    def test_parallel_sq_fail(self):
        self.assertFalse(
            check(
                [
                    [CAS(3, 5, False), CAS(7, 9, True), CAS(1, 2, True)],
                    [CAS(2, 7, True), CAS(7, 8, False), CAS(9, 10, True)],
                ],
                1
            )
        )


if __name__ == '__main__':
    unittest.main()
