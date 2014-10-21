#ifndef MATH_H_
#define MATH_H_

#include <math.h>
#include <inttypes.h>
#include <armadillo>

#include "debug.h"
#include "utils/math/Common.h"
#include "utils/math/Pose2D.h"
#include "utils/units.h"

namespace Math
{

float randGaussFloat(float mean = 0.0, float deviation = 1.0);
int16_t randGaussInt(int16_t mean = 0, int16_t deviation = 1);

/*------------------------------------------------------------------------------------------------*/

/**
 ** Limits value between min and max
 **
 ** @param val Value to limit.
 ** @param min Minimum value.
 ** @param max Maximum value.
 **
 ** @return Limited value.
 */

template <typename T1, typename T2, typename T3>
inline T1 limited(T1 val, T2 min, T3 max)
{
	ASSERT(min <= max);
	if (val < min) {
		return min;
	} else if (val > max) {
		return max;
	} else {
		return val;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Return sign of integer.
 **
 ** @param  val  integer
 **
 ** @return +1 if value is >= 0, -1 otherwise
**/

inline int getSign(int val)
{
	if (val == 0) {
		return 0;
	} else if (val < 0) {
		return -1;
	} else {
		return 1;
	}
}


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Calculate the mean of a an vector of angles (radian).
 *
 * See http://en.wikipedia.org/wiki/Mean_of_circular_quantities
 *
 * @param angles vector of angles (radian)
 *
 * @return the mean angle.
 */
inline double meanOfAngles(const arma::rowvec& angles)
{
	double tmp1(0);
	double tmp2(0);
	double weight = 1. / angles.n_elem;
	for (arma::uword i = 0; i < angles.n_elem; i++) {
		tmp1 += weight * sin(angles(i));
		tmp2 += weight * cos(angles(i));
	}
	return atan2(tmp1, tmp2);
}


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Calculate the weighted mean of a an vector of angles (radian) and weights.
 *
 * See http://en.wikipedia.org/wiki/Mean_of_circular_quantities
 *
 * @param angles vector of angles (radian)
 * @param weights the weights for each angle
 *
 * @return the mean angle.
 */
inline double meanOfAngles(const arma::rowvec& angles,
                           const arma::colvec& weights)
{
	double tmp1(0);
	double tmp2(0);
	for (arma::uword i = 0; i < angles.n_elem; i++) {
		tmp1 += weights(i) * sin(angles(i));
		tmp2 += weights(i) * cos(angles(i));
	}
	return atan2(tmp1, tmp2);
}

/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Shift the point p2 along the vector between p1 and p2 so that the
 * distance between p1 and p2 is length.
 *
 * @param p1 A point
 * @param p2 Another point which is shifted (changed!)
 * @param length the distance between p1 and p2 after the shift.
 */
inline void shiftPointAlongLineBy(const arma::colvec& p1, arma::colvec& p2, double length)
{
	// angle of the vector between p1 and p2
	arma::colvec diff = p2 - p1;
	Radian angle = atan2(diff(1), diff(0)) * radians;

	// use the seen point and the angle to create a Pose2D
	Pose2D target(angle, p1(0), p1(1));

	// now we only need to translate the given taget by length
	target.translate(length, 0.);

	// change p2 to the new target
	p2(0) = target.translation.x;
	p2(1) = target.translation.y;
};
} // end of namespace

#endif /* MATH_H_ */
