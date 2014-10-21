#ifndef REPRESENTATIONS_MOTORANGLES_H
#define REPRESENTATIONS_MOTORANGLES_H

#include <array>

#include "platform/hardware/robot/motorIDs.h"
#include "platform/system/timer.h"

class MotorAngles {
public:
	MotorAngles();

	void setValues(Millisecond timestamp,
	               const std::map<MotorID, Degree>& positions,
	               const std::map<MotorID, Degree>& offsets,
	               const std::map<MotorID, RPM>&    speeds);

	Millisecond getTimestamp() const;

	Degree  getPosition(MotorID _id) const;
	Degree  getOffset(MotorID _id)   const;
	RPM     getSpeed(MotorID _id)    const;

	const std::map<MotorID, Degree>&  getPositions() const { return positions; }
	const std::map<MotorID, Degree>&  getOffsets()   const { return offsets;   }
	const std::map<MotorID, RPM>&     getSpeeds()    const { return speeds;    }

	bool isHeadMoving() const;

	void setHeadMoving(bool _headMoving);

	/* This Function gives a good approximated diff between given MotorAngles
	 *
	 * m1 must be older then m2
	*/
	static MotorAngles getDiff(const MotorAngles& m1, const MotorAngles& m2, robottime_t t);

private:
	Millisecond timestamp; // estimated time this values were valid
	std::map<MotorID, Degree>  positions;
	std::map<MotorID, RPM>     speeds;

	std::map<MotorID, Degree>  offsets;

	bool headMoving;
};

#endif
