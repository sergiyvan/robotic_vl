/**
 * @file position.h
 *
 * Definition of the position classes.
 *
 * We have
 *         * absolute positions on the field (view coordinate system) in cm
 *         * relative positions corresponding to the current robot position
 *             (where the robot is placed in (0,0)),
 *         * image positions, which determines the position in the image in pixel and
 *         * robot positions, which are positions with the angle of vision of the robot
 *
 * The field and its coordinate system
 *
 *      y
 *      ^       ______________________
 *      |    M  |          |          |  O
 *      |    Y  |_ -x, y   |   x, y  _|  P
 *      |    G  | |        |        | |  P
 * 0    +    O  | |       ( )       | |  G
 *      |    A  |_|        |        |_|  O
 *      |    L  |  -x,-y   |   x,-y   |  A
 *      |       |__________|__________|  L
 *      |
 *      -------------------+--------------> x
 *                         0
 *
 *
 * In the relative coordinate system, x points towards and y points to the left.
 * In the polar coordinates, the angle of zero points towards, positive angles to the left
 * and negative to the right (range [-180, 180])
 *
 *
 * @{
 */

#ifndef __POSITION_H__
#define __POSITION_H__

#include "utils/math/Math.h"
#include "utils/math/Common.h"
#include "utils/units.h"

#include "ModuleFramework/Serializer.h"

#include <armadillo>
#include <inttypes.h>
#include <limits>
#include <float.h>
#include <math.h>
#include <typeinfo>


/*------------------------------------------------------------------------------------------------*/

class PositionRelative;
class PositionAbsolute;
class PositionRobot;


/*------------------------------------------------------------------------------------------------*/

/**
 ** "Abstract" position class
 */

class Position {
protected:
	Centimeter x, y;
	bool valid;

protected:

	Position()
		: x(0. * centimeters), y(0. * centimeters), valid(false) {
	}

	Position(Centimeter _x, Centimeter _y)
		: x(_x), y(_y), valid(true) {
	}

	explicit Position(arma::colvec2 posCM)
		: x(posCM(0) * centimeters), y(posCM(1) * centimeters), valid(true) {
	}

public:
	virtual ~Position() {
	}

	/**
	 * Gets the x value
	 * @return x
	 */
	inline Centimeter getX() const {
		return x;
	}

	/**
	 * Gets the y value
	 * @return y
	 */
	inline Centimeter getY() const {
		return y;
	}

	/**
	 * Sets the position
	 * @param _x new x coordinate
	 * @param _y new y coordinate
	 */
	inline void setPosition(Centimeter _x, Centimeter _y) {
		x = _x;
		y = _y;
		valid = true;
	}

	/**
	 * Sets the x coordinate
	 * @param _x
	 */
	inline void setX(Centimeter _x) {
		x = _x;
		valid = true;
	}

	/**
	 * Sets the y coordinate
	 * @param _y
	 */
	inline void setY(Centimeter _y) {
		y = _y;
		valid = true;
	}

	/**
	 * Gets the distance between this position and position p
	 * @param p Reference on the other position
	 */
	virtual Centimeter getDistance(const Position &p) const {
		// if same class
		if (typeid(*this) == typeid(p)) {
			const double distanceSq = ((x - p.x).value() * (x - p.x).value() + (y - p.y).value() * (y - p.y).value());
			return sqrt(distanceSq) * centimeters;
		}
		else {
			// this object and p aren't from the same subclass, so it's impossible to calculate the distance
			ERROR("cannot calculate distance between different position types");
			ASSERT(false);
			return 0 * centimeters;
		}
	}

	virtual Centimeter distanceToModel(const Position &m1, const Position &m2) const;

	virtual void rotateAsVectorByAngle(Degree alpha);

	inline Position operator-(const Position &p) const {
		return Position(x - p.x, y - p.y);
	}

	virtual bool isValid() const {
		return valid;
	}

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & x;
		ar & y;
		ar & valid;
	}
};

REGISTER_SERIALIZATION(Position, 1)





/*------------------------------------------------------------------------------------------------*/

/**
 * Defines absolute position on the field
 */

class PositionAbsolute: public Position {
public:
	PositionAbsolute() : Position() {
	}

	PositionAbsolute(Centimeter _x, Centimeter _y) :
		Position(_x, _y) {
	}

	virtual ~PositionAbsolute() {
	}

	/// explicit transformation
	PositionRelative translateToRelative(const PositionRobot &robotPos) const;

	// TODO: can we really add/subtract absolute positions? Or shouldn't
	//       this be adding/subtracting a relative (or polar) position
	//       to/from an absolute position only?

	PositionAbsolute operator* (const double scalar) const {
		return PositionAbsolute(getX() * scalar, getY() * scalar);
	}
	PositionAbsolute operator+ (const PositionAbsolute &pos) const {
		return PositionAbsolute(getX() + pos.getX(), getY() + pos.getY());
	}
	PositionAbsolute operator- (const PositionAbsolute &pos) const {
		return PositionAbsolute(getX() - pos.getX(), getY() - pos.getY());
	}
	PositionAbsolute& operator+= (const PositionAbsolute &pos) {
		x += pos.getX();
		y += pos.getY();
		return *this;
	}
	PositionAbsolute& operator-= (const PositionAbsolute &pos) {
		x -= pos.getX();
		y -= pos.getY();
		return *this;
	}

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		boost::serialization::base_object<Position>(*this);
	}
};

REGISTER_SERIALIZATION(PositionAbsolute, 1)

/*------------------------------------------------------------------------------------------------*/
/**
 ** Defines a relative position.
 **
 ** This means, that the robot coordinates are (0,0) and
 ** the object of interest is placed corresponding to that point
 ** the robot is looking along the positive x-axis
 ** the y-axis is directed to the left
 */
class PositionRelative
	: public Position {
public:
	PositionRelative()
		: Position() {

	}

	PositionRelative(Centimeter _x, Centimeter _y)
		: Position(_x, _y) {
	}

	explicit PositionRelative(arma::colvec2 const _posCM)
		: Position(_posCM) {
	}

	virtual ~PositionRelative() {}

	// transformation of coordinates
	PositionAbsolute translateToAbsolute(const PositionRobot& robotPos) const;

	/// get angle
	inline Radian getAngle() const {
		return atan2(getY().value(), getX().value()) * radians;
	}


	virtual inline PositionRelative operator-(const PositionRelative &p) const {
		return PositionRelative(getX() - p.getX(), getY() - p.getY());
	}

	virtual inline PositionRelative operator+(const PositionRelative &p) const {
		return PositionRelative(getX() + p.getX(), getY() + p.getY());
	}
	virtual inline PositionRelative const& operator+=(const PositionRelative &p) {
		*this = *this + p;
		return *this;
	}

	template<class T>
	inline PositionRelative operator*(const T &scalar) const {
		return PositionRelative(x*scalar, y*scalar);
	}
	template<class T>
	inline PositionRelative operator*=(const T &scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}


	// misc
	Centimeter getDistanceToMyself() const;

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		boost::serialization::base_object<Position>(*this);
	}
};

REGISTER_SERIALIZATION(PositionRelative, 1)


/*------------------------------------------------------------------------------------------------*/

/**
 ** Defines the position of a robot with the angle of view
 */

class PositionRobot: public PositionAbsolute {
protected:
	/// angle of view
	Degree angle;

public:
	PositionRobot() : PositionAbsolute() {
	}

	PositionRobot(Centimeter _x, Centimeter _y, Degree _angle)
		: PositionAbsolute(_x, _y)
		, angle(Math::normalize(_angle)) {
	}

	PositionRobot(const PositionAbsolute &pos, Degree _angle)
		: PositionAbsolute(pos.getX(), pos.getY())
		, angle(_angle) {
	}

	PositionRobot(const Pose2D &pos)
		: PositionAbsolute(pos.translation.x * centimeters, pos.translation.y * centimeters)
		, angle(Degree(Math::normalize(pos.getAngle()))) {
	}

	virtual ~PositionRobot() {
	}

	Degree getAngle() const {
		return angle;
	}

	void setAngle(Degree _angle) {
		angle = _angle;
	}

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & boost::serialization::base_object<PositionAbsolute>(*this);
		ar & angle;
	}
};

REGISTER_SERIALIZATION(PositionRobot, 1)

#endif /* __POSITION_H__ */
