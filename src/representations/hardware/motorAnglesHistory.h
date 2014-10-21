#ifndef MOTORANGLESHISTORY_H
#define MOTORANGLESHISTORY_H

#include <list>

#include "platform/system/timer.h"

#include "motorAngles.h"


/*------------------------------------------------------------------------------------------------*/

/**
 * @brief Save the current and MAXGYROHISTORY last gyro values.
 */

class MotorAnglesHistory {
public:
	void addMotorAngles(MotorAngles const& data);

	MotorAngles getMotorAngles(robottime_t timestamp) const;

protected:
	std::list<MotorAngles> motorAnglesList;
};



#endif
