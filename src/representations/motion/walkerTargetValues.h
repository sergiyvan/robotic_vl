/*
 * walkerTargetValues.h
 *
 *  Created on: Mar 22, 2014
 *      Author: stepuh
 */

#ifndef WALKERTARGETVALUES_H_
#define WALKERTARGETVALUES_H_


#include "utils/math/Math.h"





// Relative position and orientation
struct WalkerConfiguration {
	float x;
	float y;
	float yaw;
};

typedef WalkerConfiguration WalkerSpeeds;





enum class InstructionMethod {
	VIA_SPEEDS,
	VIA_CONFIGURATION,
	VIA_PATH
};



class WalkerTargetValues {
public:


	WalkerTargetValues()
		: targetConfig({0.0, 0.0, 0.0})
		, targetSpeeds({0.0, 0.0, 0.0})
		, lastInstructionMethod(InstructionMethod::VIA_SPEEDS)
	{
	}


	virtual ~WalkerTargetValues()
	{

	}


	void setTargetConfiguration(WalkerConfiguration newTargetConfig) {
		this->targetConfig = newTargetConfig;
		lastInstructionMethod = InstructionMethod::VIA_CONFIGURATION;
	}


	void setTargetConfigurationX(float targetX) {
		this->targetConfig.x = targetX;
		lastInstructionMethod = InstructionMethod::VIA_CONFIGURATION;
	}


	void setTargetConfigurationY(float targetY) {
		this->targetConfig.y = targetY;
		lastInstructionMethod = InstructionMethod::VIA_CONFIGURATION;
	}


	void setTargetConfigurationYaw(float targetYaw) {
		this->targetConfig.yaw = targetYaw;
		lastInstructionMethod = InstructionMethod::VIA_CONFIGURATION;
	}


	WalkerConfiguration getTargetConfiguration() const {
		return this->targetConfig;
	}


	void setTargetSpeeds(WalkerSpeeds newTargetSpeeds) {
		this->targetSpeeds.x   = Math::limited(newTargetSpeeds.x,   -100.0, 100.0);
		this->targetSpeeds.y   = Math::limited(newTargetSpeeds.y,   -100.0, 100.0);
		this->targetSpeeds.yaw = Math::limited(newTargetSpeeds.yaw, -100.0, 100.0);
		lastInstructionMethod = InstructionMethod::VIA_SPEEDS;
	}


	void setTargetSpeedX(float speedX) {
		this->targetSpeeds.x   = Math::limited(speedX,   -100.0, 100.0);
		lastInstructionMethod = InstructionMethod::VIA_SPEEDS;
	}


	void setTargetSpeedY(float speedY) {
		this->targetSpeeds.y   = Math::limited(speedY,   -100.0, 100.0);
		lastInstructionMethod = InstructionMethod::VIA_SPEEDS;
	}


	void setTargetSpeedYaw(float speedYaw) {
		this->targetSpeeds.yaw   = Math::limited(speedYaw,   -100.0, 100.0);
		lastInstructionMethod = InstructionMethod::VIA_SPEEDS;
	}


	WalkerSpeeds getTargetSpeeds() const {
		return this->targetSpeeds;
	}


	void setLastInstructionMethod(InstructionMethod method) {
		lastInstructionMethod = method;
	}

	InstructionMethod getLastInstructionMethod() const {
		return lastInstructionMethod;
	}


private:


	WalkerConfiguration targetConfig;         // configuration you want the robot to have
	WalkerSpeeds targetSpeeds;                // speeds you want the robot to have
	InstructionMethod lastInstructionMethod;  // how the last instruction was given
};

#endif /* WALKERTARGETVALUES_H_ */
