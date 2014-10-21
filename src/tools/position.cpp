#include "tools/position.h"

#include "utils/math/Math.h"
#include "utils/math/Vector2.h"
#include "utils/math/Common.h"

#include <limits>
#include <float.h>


/*================================================================================================*/
/* POSITION */
/*================================================================================================*/
/**
 * Calculates the distance from the current position to the line through the positions m1 and m2
 * g: x = m1 + t * u with u = (m2 - m1)
 *
 * @param m1 model point one
 * @param m2 model point two
 * @return d = | (this-m1) x u | / |u|
 */
Centimeter Position::distanceToModel(const Position &m1, const Position &m2) const {

	// types doesn't match, return max value!
	if ( ! (typeid(*this) == typeid(m1) && typeid(*this) == typeid(m2)) ) {
		// this object and p aren't from the same subclass, so it's impossible to calculate the distance
		ERROR("cannot calculate distance between different position types");
		ASSERT(false);
		return 0 * centimeters;
	}

	Centimeter ux = m2.x - m1.x;
	Centimeter uy = m2.y - m1.y;

	Centimeter pMinAX = x - m1.x;
	Centimeter pMinAY = y - m1.y;

	auto cross =  abs(pMinAX * uy - pMinAY * ux); // just z component
	Centimeter lenghtU = sqrt((ux * ux + uy * uy).value()) * centimeters;

	if (lenghtU == 0 * centimeters) {
		return cross / centimeters;
	}

	return  cross / lenghtU;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Rotate the vector {x, y} by the angle alpha in 2D
 **
 ** @param alpha Angle in degree
 */

void Position::rotateAsVectorByAngle(Degree alpha) {
	Vector2f vec(x.value(), y.value());
	vec.rotate(alpha);

	x = vec.x * centimeters;
	y = vec.y * centimeters;
}

/*================================================================================================*/
/* POSITION RELATIVE */
/*================================================================================================*/

/*------------------------------------------------------------------------------------------------*/
Centimeter PositionRelative::getDistanceToMyself() const {

	if (isValid())
		return std::sqrt((x*x + y*y).value()) * centimeters;
	else
		return std::numeric_limits<double>::max() * centimeters;
}

/*------------------------------------------------------------------------------------------------*/
/**
 * Translates the relative position (relative to the given robot position) to an absolute position on the field
 * @return absolute position on the field
 */
PositionAbsolute PositionRelative::translateToAbsolute(const PositionRobot& robotPos) const {
	if (! isValid()) {
		return PositionAbsolute();
	}

	Vector2f v(getX().value(), getY().value());
	v.rotate(robotPos.getAngle());

	Centimeter xx = v.x * centimeters + robotPos.getX();
	Centimeter yy = v.y * centimeters + robotPos.getY();

	return PositionAbsolute(xx, yy);
}

/*================================================================================================*/
/* POSITION ABSOLUTE */
/*================================================================================================*/
/**
 * Translates the absolute position to an relative position from given robot position (from localization)
 * @return relative position to robot
 */
PositionRelative PositionAbsolute::translateToRelative(const PositionRobot &robotPos) const {
	if (! isValid()) {
		return PositionRelative();
	}

	Vector2f v(getX().value() - robotPos.getX().value(), getY().value() - robotPos.getY().value());
	v.rotate(-robotPos.getAngle());
	return PositionRelative( v.x * centimeters, v.y * centimeters );
}


