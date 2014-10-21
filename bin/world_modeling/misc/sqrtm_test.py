import unittest

from sqrtm import sqrtm
from sqrtm import _assert_sqrt

from numpy import ones
from numpy import dot

class SqrtMTest(unittest.TestCase):
    def setUp(self):
        #self.state = Pose2D()
        pass
        #self.u = _control_odometry_u()

    def test_N_N_matrix(self):
        constant = 4.
        m = ones((3, 3)) * constant

        sqrt_of_m = sqrtm(m)
        self._test_equality(m, sqrt_of_m)

        m = ones((10, 10)) * 0.9
        sqrt_of_m = sqrtm(m)
        self._test_equality(m, sqrt_of_m)


    #def test_N_M_matrix(self):
        #m = ones((3, 17)) * 0.3

        #sqrt_of_m = sqrtm(m)
        #self._test_equality(m, sqrt_of_m)

    def _test_equality(self, original, sqrt_of_original):
        self.assertEqual(original.shape, sqrt_of_original.shape)
        result = dot(sqrt_of_original, sqrt_of_original)
        cols, rows = original.shape
        for i in range(cols):
            for j in range(rows):
                print 'orig', original[i, j]
                print 'sqrt', result[i, j]
                print 'absdiff', abs(original[i, j] - result[i, j])
                self.assertAlmostEqual(original[i, j], result[i, j], 1)

