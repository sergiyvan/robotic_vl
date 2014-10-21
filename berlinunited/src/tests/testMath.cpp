#include "utils/math/Math.h"
#include <gtest/gtest.h>


using namespace Math;


TEST(MathNormalizeRad, InBounds) {
	// test values that are between -pi and pi
	for (Radian r = -Math::pi*radians; r < Math::pi*radians; r += 0.01*radians) {
		Radian value = normalize(r);
		EXPECT_FLOAT_EQ(r.value(), value.value());
	}
}

TEST(MathNormalizeRad, OutBounds) {
	// test values that are NOT between -pi and pi
	for (Radian r = Math::pi*radians; r < 100*radians; r += 0.01*radians) {
		Radian value;

		value = normalize(r);
		EXPECT_TRUE(value < Math::pi*radians);
		EXPECT_TRUE(value >= -Math::pi*radians);

		value = normalize(-r);
		EXPECT_TRUE(value < Math::pi*radians);
		EXPECT_TRUE(value >= -Math::pi*radians);
	}

	EXPECT_FLOAT_EQ(-Math::pi_2, normalize( Math::pi3_2*radians).value());
	EXPECT_FLOAT_EQ( Math::pi_2, normalize(-Math::pi3_2*radians).value());
}

TEST(MathNormalizeDeg, InBounds) {
	// test values that are between -180 and 180
	for (Degree d = -180*degrees; d < 180*degrees; d += 1*degrees) {
		Degree value = normalize(d);
		EXPECT_FLOAT_EQ(d.value(), value.value());
	}
}

TEST(MathNormalizeDeg, OutBounds) {
	// test values that are NOT between -180 and 180
	for (Degree d = 180*degrees; d < 1800*degrees; d += 1*degrees) {
		Degree value;

		value = normalize(d);
		EXPECT_TRUE(value < 180*degrees);
		EXPECT_TRUE(value >= -180*degrees);

		value = normalize(-d);
		EXPECT_TRUE(value < 180*degrees);
		EXPECT_TRUE(value >= -180*degrees);
	}

	EXPECT_FLOAT_EQ(- 10, normalize( 350*degrees).value());
	EXPECT_FLOAT_EQ(  10, normalize(-350*degrees).value());
	EXPECT_FLOAT_EQ(-160, normalize( 200*degrees).value());
}

TEST(MathAngleDelta, AngleSignedDelta) {
	EXPECT_FLOAT_EQ(20, Math::angleSignedDelta(-10*degrees, 10*degrees).value());
	EXPECT_FLOAT_EQ(-20, Math::angleSignedDelta(10*degrees, -10*degrees).value());
}
