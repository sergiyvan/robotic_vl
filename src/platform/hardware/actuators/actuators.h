/*
 * actuators.h
 *
 *  Created on: June 03, 2014
 *      Author: sgg
 */

#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <map>
#include "utils/units.h"
#include "platform/hardware/robot/motorIDs.h"

class Actuators {
public:
	virtual ~Actuators() {}

	virtual bool init() { return true; }

	/// get all motor positions
	virtual std::map<MotorID, Degree>    getPositions()   const { return {}; }

	/// get offsets for the motors
	virtual std::map<MotorID, Degree>    getOffsets()     const { return {}; }

	/// get all (current) motor speeds
	virtual std::map<MotorID, RPM>       getSpeeds()      const { return {}; }

	/// get all motor data (positions/speed)
	virtual std::map<MotorID, MotorData> getMotorData()   const { return {}; }

	/// get statistics about the motors
	virtual MotorStatistics              getStatistics()  const { return {}; }

	/// set the positions and speeds of the servos
	virtual void setPositionsAndSpeeds(const std::map<MotorID, Degree> &positions, const std::map<MotorID, RPM> &speeds) {}

	/// set the servo offsets
	virtual void setOffsets(const std::map<MotorID, Degree> &motors) {}

	/// save the servo offsets
	virtual void saveOffsets() {}

	/// enable/disable the torque
	virtual void setTorqueEnabled(const std::map<MotorID, bool> &motors) {}

	/// enable/disable the servo LEDs
	virtual void setLED(const std::map<MotorID, bool> &active) {}
};

#endif
