#include <gtest/gtest.h>

#include "utils/math/Common.h"
#include "utils/math/quaternion.h"

TEST(TestQuaternion, RotateAroundZ) {

	QuaternionF q(10, 0, 0);

	QuaternionF rotateBy90 = q.rotate(Math::fromDegrees(90.), 0, 0, 1);
	EXPECT_NEAR(rotateBy90.i, 0., 0.1);
	EXPECT_NEAR(rotateBy90.j, 10., 0.1);
	EXPECT_NEAR(rotateBy90.k, 0., 0.1);

	QuaternionF rotateByMinus45 = q.rotate(Math::fromDegrees(-45.), 0, 0, 1);
	EXPECT_NEAR(rotateByMinus45.i, 10. * cos(Math::fromDegrees(-45.0)), 0.1);
	EXPECT_NEAR(rotateByMinus45.j, 10. * sin(Math::fromDegrees(-45.0)), 0.1);
	EXPECT_NEAR(rotateByMinus45.k, 0., 0.1);
}

TEST(TestQuaternion, RotateAroundX) {

	QuaternionF q(0, 10, 0);
	QuaternionF rotateBy90 = q.rotate(Math::fromDegrees(90.), 1, 0, 0);
	EXPECT_NEAR(rotateBy90.i, 0., 0.1);
	EXPECT_NEAR(rotateBy90.j, 0., 0.1);
	EXPECT_NEAR(rotateBy90.k, 10., 0.1);

	QuaternionF rotateByMinus45 = q.rotate(Math::fromDegrees(-45.), 1, 0, 0);
	EXPECT_NEAR(rotateByMinus45.i, 0., 0.1);
	EXPECT_NEAR(rotateByMinus45.j, 10. * cos(Math::fromDegrees(-45.0)), 0.1);
	EXPECT_NEAR(rotateByMinus45.k, 10. * sin(Math::fromDegrees(-45.0)), 0.1);
}

TEST(TestQuaternion, RotateAroundY) {

	QuaternionF q(0, 0, 10);
	QuaternionF rotateBy90 = q.rotate(Math::fromDegrees(90.), 0, 1, 0);
	EXPECT_NEAR(rotateBy90.i, 10., 0.1);
	EXPECT_NEAR(rotateBy90.j, 0., 0.1);
	EXPECT_NEAR(rotateBy90.k, 0., 0.1);

	QuaternionF rotateByMinus45 = q.rotate(Math::fromDegrees(-45.), 0, 1, 0);
	EXPECT_NEAR(rotateByMinus45.i, 10. * sin(Math::fromDegrees(-45.0)), 0.1);
	EXPECT_NEAR(rotateByMinus45.j, 0., 0.1);
	EXPECT_NEAR(rotateByMinus45.k, 10. * cos(Math::fromDegrees(-45.0)), 0.1);
}

