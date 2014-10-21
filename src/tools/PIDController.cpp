/*
 * PIDController.cpp
 *
 *  Created on: May 22, 2012
 *      Author: Simon Philipp Hohberg
 */

#include "PIDController.h"
#include "debug.h"

float PIDController::calculate(float is)
{
	robottime_t currentTime = getCurrentTime();
	robottime_t deltaT = currentTime - lastTime;

	float error = desired - is;

	float p = kp*error;
	float result =  p;

	if (hasD) {
		float d = kd*((error - lastError)/deltaT.value());
		result += d;
	}

	if (hasI) {
		integral += error*deltaT.value();
		float i = ki*integral;
		result += i;
	}

	lastError = error;
	lastTime = currentTime;

	return result;
}



