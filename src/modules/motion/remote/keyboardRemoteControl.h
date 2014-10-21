#ifndef __KEYBOARDREMOTECONTROL_H__
#define __KEYBOARDREMOTECONTROL_H__

#include "ModuleFramework/Module.h"

#include "representations/hardware/motorAngles.h"
#include "representations/motion/motionRequest.h"
#include "representations/motion/motionStart.h"
#include "representations/motion/motionStatus.h"
#include "representations/motion/motorPositionRequest.h"
#include "representations/motion/walkerTargetValues.h"
//#include "representations/posture.h"

#include "platform/system/timer.h"


BEGIN_DECLARE_MODULE(KeyboardRemoteControl)
	REQUIRE(MotionStart)
//	REQUIRE(RobotPosture)
	REQUIRE(MotorAngles)

	PROVIDE(MotorPositionRequest)
	PROVIDE(MotionRequest)
	PROVIDE(MotionStatus)
	PROVIDE(WalkerTargetValues)
END_DECLARE_MODULE(KeyboardRemoteControl)


class KeyboardRemoteControl : public KeyboardRemoteControlBase {
private:
	void printInstructions();

	robottime_t lastSpeedInformation;
	uint8_t disableMotors;
	bool allowHeadMovements;

public:
	KeyboardRemoteControl();
	virtual ~KeyboardRemoteControl();

	virtual void init() override;
	virtual void execute() override;
};

#endif
