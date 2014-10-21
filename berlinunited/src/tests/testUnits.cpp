#include <gtest/gtest.h>
#include "utils/units.h"

#include "utils/math/Math.h"

class TestUnits : public ::testing::Test {
protected:
	virtual void SetUp() {
	}
};

TEST_F(TestUnits, TestTypeSafety) {
	// if this fails, it is actually failing at compile-time
	Millimeter m = 10*millimeters;

	EXPECT_EQ(10, m.value());
}

TEST_F(TestUnits, TestUnitConversionLength) {
	Meter m = 10*meters;

	EXPECT_EQ(1000, Centimeter(m).value());
	EXPECT_EQ(10000, Millimeter(m).value());

	EXPECT_EQ(15, (1.5 * m).value());
}

TEST_F(TestUnits, TestMixedUnitOperationsLength) {
	Meter m = 1*meters;
	Centimeter cm = 1*centimeters;
	Millimeter mm = 1*millimeters;

	EXPECT_DOUBLE_EQ(1011, (m + cm + mm).value());
	EXPECT_DOUBLE_EQ(1011, (m + mm + cm).value());
	EXPECT_DOUBLE_EQ(1011, (cm + m + mm).value());
	EXPECT_DOUBLE_EQ(1011, (cm + mm + m).value());
	EXPECT_DOUBLE_EQ(1011, (mm + cm + m).value());
	EXPECT_DOUBLE_EQ(1011, (mm + m + cm).value());
}

TEST_F(TestUnits, TestUnitConversionAngles) {
	Degree deg = 180*degrees;
	EXPECT_FLOAT_EQ(Math::pi, Radian(deg).value());

	Radian rad = Math::pi / 2 * radians;
	EXPECT_FLOAT_EQ(90, Degree(rad).value());
}

TEST_F(TestUnits, TestMixedUnitOperationsAngles) {
	Radian rad = Math::pi * radians;
	Degree deg = 90 * degrees;

	EXPECT_FLOAT_EQ(Math::pi*3/2, (rad + deg).value());
	EXPECT_FLOAT_EQ(Math::pi*3/2, (deg + rad).value());
}

TEST_F(TestUnits, TestScalingAngles) {
	Degree deg = 45 * degrees;
	deg *= 3;

	EXPECT_FLOAT_EQ(135.0, deg.value());
}

TEST_F(TestUnits, TestTrigonometryWithDegrees) {
	Degree deg = 90 * degrees;

	EXPECT_FLOAT_EQ(1.0, sin(deg).value());
}

TEST_F(TestUnits, TestScalingLengths) {
}

TEST_F(TestUnits, TestDegreesPerSecond) {
	// if this fails, it fails at compile time
	DPS av = (5*degrees) / (1*seconds);

	EXPECT_FLOAT_EQ(5.0, av.value());
}

TEST_F(TestUnits, TestRadiansPerSecond) {
	// if this fails, it fails at compile time
	RPS av = (1*radians) / (1*seconds);

	EXPECT_FLOAT_EQ(1.0, av.value());
}

TEST_F(TestUnits, TestTimeConversion) {
	Microsecond us = 500*microseconds;
	EXPECT_FLOAT_EQ(0.5, Millisecond(us).value());

	Millisecond ms = 1000*milliseconds;
	EXPECT_FLOAT_EQ(1, Second(ms).value());

	Second s = 0.1*seconds;
	EXPECT_FLOAT_EQ(100, Millisecond(s).value());
}

TEST_F(TestUnits, TestHertzFrequencyCalculation) {
	double iterations = 50;
	Second time = 10*seconds;
	Hertz hz = iterations/time;

	EXPECT_FLOAT_EQ(5.0, hz.value());
}

TEST_F(TestUnits, TestHertzFrequencyCalculationInteger) {
	int iterations = 100;
	Second time = 2*seconds;
	Hertz hz = iterations/time;

	EXPECT_FLOAT_EQ(50.0, hz.value());
}

TEST_F(TestUnits, TestHertzFrequencyInverse) {
	Hertz  h = 20*hertz;
	Second s = 1.0/h;
	EXPECT_FLOAT_EQ(0.05, s.value());
}

TEST_F(TestUnits, TestRPMFrequencyCalculation) {
	double iterations = 50;
	Minute time = 10*minutes;
	RPM r = iterations/time;

	EXPECT_FLOAT_EQ(5.0, r.value());
}

TEST_F(TestUnits, TestRPMFrequencyCalculationInteger) {
	int iterations = 100;
	Minute time = 2*minutes;
	RPM r = iterations/time;

	EXPECT_FLOAT_EQ(50.0, r.value());
}

TEST_F(TestUnits, TestRPMFrequencyInverse) {
	RPM    r = 20*rounds_per_minute;
	Minute m = 1.0/r;
	EXPECT_FLOAT_EQ(0.05, m.value());
}

TEST_F(TestUnits, TestRPMandHzConversion) {
	RPM r = 20*rounds_per_minute;
	EXPECT_FLOAT_EQ(20./60, Hertz(r).value());

	Hertz h = 5*hertz;
	EXPECT_FLOAT_EQ(5*60, RPM(h).value());
}
