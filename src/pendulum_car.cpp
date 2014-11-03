/**
 * @file
 *
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

#include "tools/PIDController.h"

#include <sstream>
#include <iostream>



/*------------------------------------------------------------------------------------------------*/
BEGIN_DECLARE_MODULE(PendulumCarMotionTest)
	REQUIRE(MotorAngles)
	PROVIDE(MotorPositionRequest)
END_DECLARE_MODULE(PendulumCarMotionTest)

class PendulumCarMotionTest : public PendulumCarMotionTestBase {
private:
	Degree lastAngle;
    Second lastTime;

	std::ofstream myfile;
	Second startTime;
	Second writeTimer;


public:
	PendulumCarMotionTest() {
	}

	virtual void init() {
        RobotDescription const& robotDescription = *services.getRobotModel().getRobotDescription();
        MotorID idP = robotDescription.getEffectorID("pendulum");
        lastAngle = getMotorAngles().getPosition(idP);
        lastTime = 0 * seconds;

		myfile.open("angle_values.csv");
        startTime = Second(getCurrentTime());
		writeTimer = 0 * seconds;

	}

	virtual void execute() {
        RobotDescription const& robotDescription = *services.getRobotModel().getRobotDescription();
        MotorID idL = robotDescription.getEffectorID("wheelL");
        MotorID idR = robotDescription.getEffectorID("wheelR");
        MotorID idP = robotDescription.getEffectorID("pendulum");

        RPM curSpeedL = getMotorAngles().getSpeed(idL);
        INFO("current wheel speed: %f", curSpeedL.value());

        Degree curAngle = getMotorAngles().getPosition(idP);
        INFO("current pendulum angle: %f", curAngle.value());

        Second curTime = Second(getCurrentTime())-startTime;
        INFO("time since start (sec): %f", curTime.value());

        Second timeDiff = curTime - lastTime;
        INFO("time since last update (sec): %f", timeDiff.value());

        Degree angleDiff = curAngle - lastAngle;
        DPS dps = angleDiff/timeDiff;
        INFO("pendulum angular rate: %f", dps.value());

        INFO("===============================");


        //set speed value (insert your code here)
        RPM speed = 0. * rounds_per_minute;

        getMotorPositionRequest().setSpeed(idL, speed);
        getMotorPositionRequest().setSpeed(idR, speed);

        //update lastValues
        lastAngle = curAngle;
        lastTime = curTime;

        //output in file (every ~0.1 seconds)
        writeTimer += timeDiff;
        if (writeTimer > 0.1 * seconds) {
            myfile << curTime.value() << ","
            << curAngle.value()
            << std::endl;
            //reset timer
            writeTimer = 0 * seconds;
        }



	}
};



class PendulumCarTestCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled("PendulumCarMotionTest", true);

		// run managers and wait for termination - comment this line if you want to quit right away
		services.runManagers();

		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion,    PendulumCarMotionTest,    false, "Test module from car.cpp")

namespace {
	auto cmdTest = CommandLine::getInstance().registerCommand<PendulumCarTestCmdLineCallback>(
			"pendulum_car",
			"Robot PendulumCar Test",
			ModuleManagers::none()->enable<Motion>() );
}
