from ukfloc2 import _control_odometry_u
import unittest

from helper_math import Pose2D
from helper_math import normalize_radians

from numpy import radians


class Pose2DTest(unittest.TestCase):
    def setUp(self):
        self.state = Pose2D()
        self.u = _control_odometry_u()

    def test_forward(self):
        self.u.x = 10

        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 10.)
        self.assertEqual(self.state.y, 0.)
        self.assertEqual(self.state.rot, 0)

        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 20.)
        self.assertEqual(self.state.y, 0.)
        self.assertEqual(self.state.rot, 0)

        self.u.x = 5
        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 25.)
        self.assertEqual(self.state.y, 0.)
        self.assertEqual(self.state.rot, 0)

    def test_sideward(self):
        self.u.y = 10.
        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 0.)
        self.assertEqual(self.state.y, 10.)
        self.assertEqual(self.state.rot, 0)

        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 0.)
        self.assertEqual(self.state.y, 20.)
        self.assertEqual(self.state.rot, 0)

        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 0.)
        self.assertEqual(self.state.y, 30.)
        self.assertEqual(self.state.rot, 0)

    def test_rotation(self):
        self.u.rot = 10.

        # rotate
        self.state.rotate_and_translate(self.u)

        self.assertEqual(self.state.x, 0.)
        self.assertEqual(self.state.y, 0.)
        self.assertEqual(self.state.rot, normalize_radians(10.))

        # rotate
        self.state.rotate_and_translate(self.u)

        self.assertEqual(self.state.x, 0.)
        self.assertEqual(self.state.y, 0.)
        self.assertEqual(self.state.rot, normalize_radians(20))

    def test_rotation2(self):
        self.u.rot = .1

        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.rot, .1, 3)

        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.rot, .2, 3)

        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.rot, .3, 3)

        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.rot, .4, 3)

    def test_forward_then_sideward(self):
        self.u.x = 10.
        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 10.)
        self.assertEqual(self.state.y, 0.)
        self.assertEqual(self.state.rot, 0)

        self.u.x = 0.
        self.u.y = 10.
        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 10.)
        self.assertEqual(self.state.y, 10.)
        self.assertEqual(self.state.rot, 0)

    def test_forward_and_sideward2(self):
        self.u.x = 10.
        self.u.y = 10.
        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 10.)
        self.assertEqual(self.state.y, 10.)
        self.assertEqual(self.state.rot, 0)

        self.state.rotate_and_translate(self.u)
        self.assertEqual(self.state.x, 20.)
        self.assertEqual(self.state.y, 20.)
        self.assertEqual(self.state.rot, 0)

    def test_forward_sideward_rotation(self):
        self.u.x = 10.
        self.u.y = 10.
        self.u.rot = radians(180)
        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.x, 10., 1)
        self.assertAlmostEqual(self.state.y, 10., 1)
        self.assertEqual(self.state.rot, -radians(180))

        self.u = _control_odometry_u(0, 0, radians(-180))
        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.x, 10., 1)
        self.assertAlmostEqual(self.state.y, 10., 1)
        self.assertEqual(self.state.rot, radians(0))

        self.u = _control_odometry_u(0, 0, radians(90))
        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.x, 10., 1)
        self.assertAlmostEqual(self.state.y, 10., 1)
        self.assertEqual(self.state.rot, radians(90))

        self.state.rotate_and_translate(self.u)
        self.assertAlmostEqual(self.state.x, 10., 1)
        self.assertAlmostEqual(self.state.y, 10., 1)
        self.assertEqual(self.state.rot, -radians(180))
