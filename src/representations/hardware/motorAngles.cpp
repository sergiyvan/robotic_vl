#include "motorAngles.h"

#include "debug.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

MotorAngles::MotorAngles()
	 : timestamp(0)
	 , headMoving(false)
{
}


/*------------------------------------------------------------------------------------------------*/

void MotorAngles::setValues(Millisecond _timestamp,
                            const std::map<MotorID, Degree>&  _positions,
                            const std::map<MotorID, Degree>&  _offsets,
                            const std::map<MotorID, RPM>&     _speeds)
{
	timestamp = _timestamp;
	for (auto const& e : _positions) {
		positions[e.first] = e.second;
	}
	for (auto const& e : _offsets) {
		offsets[e.first] = e.second;
	}
	for (auto const& e : _speeds) {
		speeds[e.first] = e.second;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Millisecond MotorAngles::getTimestamp() const {
	return timestamp;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Degree MotorAngles::getPosition(MotorID _id) const {
	const auto& it = positions.find(_id);
	if (it != positions.end()) {
		return it->second;
	} else {
		ERROR("Unknown motor %d", (int)_id);
		return 0*degrees;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Degree MotorAngles::getOffset(MotorID id) const {
	const auto &it = offsets.find(id);
	if (it != offsets.end())
		return it->second;
	else
		return 0*degrees;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

RPM MotorAngles::getSpeed(MotorID id) const {
	const auto &it = speeds.find(id);
	if (it != speeds.end())
		return it->second;
	else
		return 0*rounds_per_minute;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool MotorAngles::isHeadMoving() const {
	return headMoving;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void MotorAngles::setHeadMoving(bool _headMoving) {
	headMoving = _headMoving;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

MotorAngles MotorAngles::getDiff(const MotorAngles& m1, const MotorAngles& m2, robottime_t t) {
	ASSERT(m1.getTimestamp() <= m2.getTimestamp());

	if (m1.getTimestamp() >= t) return m1;
	if (m2.getTimestamp() <= t) return m2;

	robottime_t timeDiff = m2.getTimestamp() - m1.getTimestamp();
	robottime_t timeRel  = t - m1.getTimestamp();

	float f2 = timeDiff / timeRel;
	float f1 = 1 - f2;

	std::map<MotorID, Degree>  p;
	std::map<MotorID, Degree>  o;
	std::map<MotorID, RPM>     s;
	for (auto const& e : m1.positions) {
		p[e.first] = e.second*f1 + m2.getPosition(e.first)*f2;
	}
	for (auto const& e : m1.offsets) {
		o[e.first] = e.second;
	}
	for (auto const& e : m1.speeds) {
		s[e.first] = e.second*f1 + m2.getSpeed(e.first)*f2;
	}

	MotorAngles m;
	m.setValues(t, p, o, s);
	return m;
}

