/**
 * @file
 *
 * This file can be used to add routines to test your new code.
 *
 * Please don't commit this file. The tests should stay on your computer.
 */

#include "debug.h"
#include "services.h"

#include "communication/comm.h"
#include "management/commandLine.h"

// module framework
#include "modules/motion/motion.h"

// representations
#include "representations/motion/motorPositionRequest.h"
#include "representations/hardware/motorAngles.h"

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

#include <sstream>
#include <iostream>



/*------------------------------------------------------------------------------------------------*/
BEGIN_DECLARE_MODULE(ArmMotionTest)
	REQUIRE(MotorAngles)
	PROVIDE(MotorPositionRequest)
END_DECLARE_MODULE(ArmMotionTest)

class ArmMotionTest : public ArmMotionTestBase {
private:

public:
	ArmMotionTest() {
	}

	virtual void init() {
	}

	virtual void execute() {
        RobotDescription const& robotDescription = *services.getRobotModel().getRobotDescription();
        MotorID id1 = robotDescription.getEffectorID("motor1");
        MotorID id2 = robotDescription.getEffectorID("motor2");

        Degree curAngle1 = getMotorAngles().getPosition(id1);
        Degree curAngle2 = getMotorAngles().getPosition(id2);

        getMotorPositionRequest().setPositionAndSpeed(id1, curAngle1 + 5. * degrees, 20. * rounds_per_minute);
        getMotorPositionRequest().setPositionAndSpeed(id2, curAngle2 + 5. * degrees, 20. * rounds_per_minute);
	}
};



class ArmTestCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled("ArmMotionTest", true);

		// run managers and wait for termination - comment this line if you want to quit right away
		services.runManagers();

		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion,    ArmMotionTest,    false, "Test module from arm.cpp")

namespace {
	auto cmdTest = CommandLine::getInstance().registerCommand<ArmTestCmdLineCallback>(
			"arm",
			"Robot Arm Test",
			ModuleManagers::none()->enable<Motion>() );
}
