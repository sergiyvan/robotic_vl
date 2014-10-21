#include "utils/math/Math.h"

#include <gtest/gtest.h>

/*------------------------------------------------------------------------------------------------*/
TEST(Test_shiftPointAlongLineBy, OnTheRight)
{
	arma::colvec2 p1;
	p1 << 200 << 50. << arma::endr;

	arma::colvec2 p2;
	p2 << 200. << -50. << arma::endr;

	Math::shiftPointAlongLineBy(p1, p2, 150.f);
	EXPECT_EQ(p2(0), 200.f);
	EXPECT_EQ(p2(1), -100.f);

	Math::shiftPointAlongLineBy(p1, p2, 100.f);
	EXPECT_EQ(p2(0), 200.f);
	EXPECT_EQ(p2(1), -50.f);

	Math::shiftPointAlongLineBy(p1, p2, 200.f);
	EXPECT_EQ(p2(0), 200.f);
	EXPECT_EQ(p2(1), -150.f);
}

/*------------------------------------------------------------------------------------------------*/
TEST(Test_shiftPointAlongLineBy, InFront)
{
	arma::colvec2 p1;
	p1 << 200 << 0. << arma::endr;

	arma::colvec2 p2;
	p2 << 300. << 0. << arma::endr;

	Math::shiftPointAlongLineBy(p1, p2, 150.f);
	EXPECT_EQ(p2(0), 350.f);
	EXPECT_EQ(p2(1), 0.f);

	Math::shiftPointAlongLineBy(p1, p2, 200.f);
	EXPECT_EQ(p2(0), 400.f);
	EXPECT_EQ(p2(1), 0.f);

	Math::shiftPointAlongLineBy(p1, p2, 50.f);
	EXPECT_EQ(p2(0), 250.f);
	EXPECT_EQ(p2(1), 0.f);
}
