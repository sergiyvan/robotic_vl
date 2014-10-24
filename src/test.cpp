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
#include "platform/hardware/robot/robotModel.h"

// module framework
#include "modules/motion/motion.h"

// representations
#include "representations/motion/motorPositionRequest.h"
#include "representations/motion/kinematicTree.h"

#include "representations/motion/kinematicengine/kinematicEngineTasks.h"

#include "tools/kinematicEngine/tasks/kinematicEngineTaskCOMWholeRobot.h"
#include "tools/kinematicEngine/tasks/kinematicEngineTaskLocation.h"

#include <sstream>
#include <iostream>


/*------------------------------------------------------------------------------------------------*/
// ------ DO NOT COMMIT THIS FILE!
// ------ It should look untouched all the time!
/*------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
BEGIN_DECLARE_MODULE(MotionTest)
	PROVIDE(KinematicEngineTasks)
	PROVIDE(MotorPositionRequest)
END_DECLARE_MODULE(MotionTest)

class MotionTest : public MotionTestBase {
private:

public:

	MotionTest() {
	}

	virtual void init() {
	}

	virtual void execute() {
		// ------ INSERT YOUR code here BUT DO NOT COMMIT THIS FILE!
	}
};


/*------------------------------------------------------------------------------------------------*/

class TestCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		INFO("TEST ROUTINE STARTED");

		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled("MotionTest", true);


		// ------ INSERT YOUR code here BUT DO NOT COMMIT THIS FILE!

		// run managers and wait for termination - comment this line if you want to quit right away
		services.runManagers();

		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion,    MotionTest,    false, "Test module from test.cpp")

namespace {
	auto cmdTest = CommandLine::getInstance().registerCommand<TestCmdLineCallback>(
			"test",
			"Test function",
			ModuleManagers::none()->enable<Motion>() );
}
