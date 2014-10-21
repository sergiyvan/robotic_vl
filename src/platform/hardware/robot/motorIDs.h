#ifndef MOTORIDS_H
#define MOTORIDS_H

#include <set>
#include <map>
#include <string>

#include "utils/units.h"


/*------------------------------------------------------------------------------------------------*/

#define MOTOR_NONE      (-0xDEADBEEF)


/*------------------------------------------------------------------------------------------------*/

typedef int32_t MotorID;
typedef std::set<MotorID>           Motors;
typedef std::map<MotorID, bool>     MotorValuesBool;
typedef std::map<MotorID, uint8_t>  MotorValues8;
typedef std::map<MotorID, uint16_t> MotorValues16;
typedef std::map<MotorID, uint32_t> MotorValues32;
typedef std::map<MotorID, Degree>   MotorPositions;
typedef std::map<MotorID, RPM>      MotorSpeeds;


/*------------------------------------------------------------------------------------------------*/

/** The MotorData class represents the current position and speed of a servo.
 */
class MotorData {
public:
	MotorData(int value=0)
		: position(value*degrees)
		, speed(value*rounds_per_minute)
	{}

	Degree        position;
	RPM           speed;
};

/** MotorStatistic provides information about the error/success rates reading
 ** the servo values.
 */
class MotorStatistic {
public:
	MotorStatistic()
		: successfulReads(0)
		, missedReads(0)
		, consecutiveFailedReads(0)
		, error(0)
	{}

	uint32_t successfulReads;
	uint32_t missedReads;
	uint32_t consecutiveFailedReads;
	uint8_t  error;                   // TODO: define

	void setSuccess() {
		successfulReads++;
		consecutiveFailedReads = 0;
	}

	void setFailure() {
		consecutiveFailedReads++;
		missedReads++;
	}
};

typedef std::map<MotorID, MotorStatistic>   MotorStatistics;


#endif
