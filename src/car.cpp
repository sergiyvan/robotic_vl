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
BEGIN_DECLARE_MODULE(CarMotionTest)
	REQUIRE(MotorAngles)
	PROVIDE(MotorPositionRequest)
END_DECLARE_MODULE(CarMotionTest)

class CarMotionTest : public CarMotionTestBase {
private:

public:
	CarMotionTest() {
	}

	virtual void init() {
	}

	virtual void execute() {
        RobotDescription const& robotDescription = *services.getRobotModel().getRobotDescription();
        MotorID id0 = robotDescription.getEffectorID("wheel0");
        MotorID id1 = robotDescription.getEffectorID("wheel1");
        MotorID id2 = robotDescription.getEffectorID("wheel2");
        MotorID id3 = robotDescription.getEffectorID("wheel3");

        MotorID steerL = robotDescription.getEffectorID("steeringL");
        MotorID steerR = robotDescription.getEffectorID("steeringR");

        getMotorPositionRequest().setSpeed(id0, 20. * rounds_per_minute);
        getMotorPositionRequest().setSpeed(id1, 20. * rounds_per_minute);
        getMotorPositionRequest().setSpeed(id2, 20. * rounds_per_minute);
        getMotorPositionRequest().setSpeed(id3, 20. * rounds_per_minute);

        getMotorPositionRequest().setPositionAndSpeed(steerL, 10 * sin(getCurrentTime().value() * degrees / 10) * degrees, 10 * rounds_per_minute);
        getMotorPositionRequest().setPositionAndSpeed(steerR, 10 * sin(getCurrentTime().value() * degrees / 10) * degrees, 10 * rounds_per_minute);
	}
};



class CarTestCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled("CarMotionTest", true);

		// run managers and wait for termination - comment this line if you want to quit right away
		services.runManagers();

		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion,    CarMotionTest,    false, "Test module from car.cpp")

namespace {
	auto cmdTest = CommandLine::getInstance().registerCommand<CarTestCmdLineCallback>(
			"car",
			"Robot Car Test",
			ModuleManagers::none()->enable<Motion>() );
}
