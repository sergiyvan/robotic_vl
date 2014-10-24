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
#include "representations/motion/kinematicTree.h"

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

#include <sstream>
#include <iostream>



/*------------------------------------------------------------------------------------------------*/
BEGIN_DECLARE_MODULE(QuadcopterTest)
	REQUIRE(KinematicTree)
	REQUIRE(MotorAngles)
	PROVIDE(MotorPositionRequest)
END_DECLARE_MODULE(QuadcopterTest)

class QuadcopterTest : public QuadcopterTestBase {
private:

public:
	QuadcopterTest() {
	}

	virtual void init() {
	}

	virtual void execute() {
		KinematicMass robotMass = getKinematicTree().getCOM(getKinematicTree().getRootNode()->getID());

        RobotDescription const& robotDescription = *services.getRobotModel().getRobotDescription();
        MotorID id0 = robotDescription.getEffectorID("propeller0");
        MotorID id1 = robotDescription.getEffectorID("propeller1");
        MotorID id2 = robotDescription.getEffectorID("propeller2");
        MotorID id3 = robotDescription.getEffectorID("propeller3");

        // rotation calculation as follows:
        // the speedToForceFactor is 0.001 which means that when the propeller is moving at 1000 rounds per minute it generates 1N force along its rotation axis
        // the gravitational pull is -9.80665 (see physicsenvironment.h) so we multiply (on average) with 10 to get an acceleration of 10m/s^2
        // propeller0 and propeller2 rotate in the oposite direction of the other rotors to make the overall quadcopter not spin around itself but I want it to spin so I have put some more speed on some propellers while removing some of others
        // one propeller gets some more speed to make it fly a bit so the side (while spinning) for the overall effect
        // we have 4 rotors so divide by 4
        getMotorPositionRequest().setSpeed(id0, robotMass.m_massGrams * rounds_per_minute * 9.1 / 4);
        getMotorPositionRequest().setSpeed(id1, robotMass.m_massGrams * rounds_per_minute * 11 / 4);
        getMotorPositionRequest().setSpeed(id2, robotMass.m_massGrams * rounds_per_minute * 9 / 4);
        getMotorPositionRequest().setSpeed(id3, robotMass.m_massGrams * rounds_per_minute * 11 / 4);
	}
};



class QuadcopterTestCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled("QuadcopterTest", true);

		// run managers and wait for termination - comment this line if you want to quit right away
		services.runManagers();

		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion,    QuadcopterTest,    false, "Test module from arm.cpp")

namespace {
	auto cmdTest = CommandLine::getInstance().registerCommand<QuadcopterTestCmdLineCallback>(
			"quadcopter",
			"Quadcopter Test",
			ModuleManagers::none()->enable<Motion>() );
}
