#include "platform/system/timer.h"

/*
 * PIDController.h
 *
 *  Created on: May 22, 2012
 *      Author: Simon Philipp Hohberg
 */

#ifndef PIDCONTROLLER_H_
#define PIDCONTROLLER_H_

class PIDController {
public:

	PIDController(float desired, float kp, float kd, float ki) :
		desired(desired),
		lastError(0),
		integral(0),
		kp(kp),
		hasD(true),
		kd(kd),
		hasI(true),
		ki(ki)
		{
			lastTime = getCurrentTime();
		};

	PIDController(float desired, float kp, float kd) :
		desired(desired),
		lastError(0),
		integral(0),
		kp(kp),
		hasD(true),
		kd(kd),
		hasI(false),
		ki(0)
		{
			lastTime = getCurrentTime();
		};

	PIDController(float desired, float kp) :
		desired(desired),
		lastError(0),
		integral(0),
		kp(kp),
		hasD(false),
		kd(0),
		hasI(false),
		ki(0)
		{
			lastTime = getCurrentTime();
		};

	void setKp(float kp) {
		this->kp = kp;
	}

	void setKd(float kd) {
		this->kd = kd;
		if (kd > 0)
			hasD = true;
		else
			hasD = false;
	}

	void setKi(float ki) {
		this->ki = ki;
		if (ki > 0)
			hasI = true;
		else
			hasI = false;
	}

	void setDesired(float desired) {
		this->desired = desired;
	}

	float calculate(float is);

protected:
	float desired;
	float lastError;
	float integral;
	robottime_t lastTime;

	float kp;

	bool hasD;
	float kd;

	bool hasI;
	float ki;
};

#endif /* PIDCONTROLLER_H_ */
