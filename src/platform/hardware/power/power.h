/*
 * power.h
 *
 *  Created on: May 13, 2014
 *      Author: dseifert
 */

#ifndef POWER_H_
#define POWER_H_

#include "utils/units.h"

class Power {
public:
	virtual ~Power() {}

	virtual bool init() { return true; }

	/// turn off power to the robot
	virtual void switchRobotOff() {}

	/// turn off power to the actuators
	virtual void switchActuatorsOff() {}

	/// turn on power to the actuators
	virtual void switchActuatorsOn() {}

	/// return true iff the actuators have power
	virtual bool hasPowerToActuators() const {
		return true;
	}

	/// return the remaining battery capacity in percent [0,100]
	virtual double getBatteryPercent() const {
		return 100.0;
	}

	/// return the current battery voltage
	virtual Volt getBatteryVoltage() const {
		return 0*volts;
	}
};

#endif

