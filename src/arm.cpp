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
#include "representations/hardware/motorAngles.h"
#include "representations/motion/kinematicTree.h"
#include "representations/motion/motorPositionRequest.h"

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

//io
#include <iostream>
#include <fstream>
#define PI 3.14159265
using namespace std;
/*------------------------------------------------------------------------------------------------*/
BEGIN_DECLARE_MODULE(ArmMotionTest)
REQUIRE(MotorAngles)REQUIRE(KinematicTree)PROVIDE(MotorPositionRequest)END_DECLARE_MODULE(ArmMotionTest)

class ArmMotionTest: public ArmMotionTestBase {
private:
	std::ofstream myfile;
	int state;
	std::pair<float, float> goals[4];

public:
	ArmMotionTest() {
	}

	virtual void init() {
		myfile.open("motor_values.csv");
		state = 0;
		goals[0] = std::make_pair(0.2, 0.7);
		goals[1] = std::make_pair(-0.3, 0.7);
		goals[2] = std::make_pair(-0.2, 0.3);
		goals[3] = std::make_pair(0.3, 0.2);
	}

	virtual void execute() {

		RobotDescription const& robotDescription =
				*services.getRobotModel().getRobotDescription();
		MotorID idRoot = robotDescription.getEffectorID("root");
//        MotorID id0 = robotDescription.getEffectorID("motor0");
		MotorID id1 = robotDescription.getEffectorID("motor1");
		MotorID id2 = robotDescription.getEffectorID("motor2");
		MotorID idHand = robotDescription.getEffectorID("hand");

//        Degree curAngle0 = getMotorAngles().getPosition(id0);
//        Degree curAngle1 = getMotorAngles().getPosition(id1);
//        Degree curAngle2 = getMotorAngles().getPosition(id2);

		arma::mat44 transMatRootToHand =
				getKinematicTree().getTransitionMatrixFromTo(idRoot, idHand);

		//replace this dummy code with your code

		uint64_t time = getCurrentTime().value();
		state = (time / 4000) % 4; //change state (modulo 4) every 4000 milliseconds
		INFO("State: %d", state)

		Degree angle1, angle2;

		anglesFromYZ(angle1, angle2, goals[state].first, goals[state].second);

		getMotorPositionRequest().setPositionAndSpeed(id1, angle1,
				40. * rounds_per_minute);
		getMotorPositionRequest().setPositionAndSpeed(id2, angle2,
				40. * rounds_per_minute);

		//output on console
		INFO("x: %f y: %f z: %f", transMatRootToHand(0, 3),
				transMatRootToHand(1, 3), transMatRootToHand(2, 3));
//        INFO("\n(%f, %f, %f)\n(%f, %f, %f)\n(%f, %f, %f)",
//        		transMatRootToHand(0,0), transMatRootToHand(0,1), transMatRootToHand(0,2),
//        		transMatRootToHand(1,0), transMatRootToHand(1,1), transMatRootToHand(1,2),
//        		transMatRootToHand(2,0), transMatRootToHand(2,1), transMatRootToHand(2,2)
//        );

//output in file
		myfile << transMatRootToHand(0, 3) << "," << transMatRootToHand(1, 3)
				<< "," << transMatRootToHand(2, 3) << "," << std::endl;
	}

	bool anglesFromYZ(Degree& angle1, Degree& angle2, float y, float z) {
		float b = 0.25; //edge between motor2 and hand
		float a = 0.2; //edge between motor1 and motor2
		float mod_z = z - 0.5;
		float c = sqrt(y * y + mod_z * mod_z);
		float alpha = acos((y * y + c * c - mod_z * mod_z) / (2 * y * c))
				* 180.0 / PI;
		cout << "y: " << y << '\n';
		cout << "z: " << z << '\n';
		cout << "c: " << c << '\n';
		cout << "alpha:" << alpha << '\n';
		float val_1 = (c * c + a * a - b * b) / (2 * a * c); //beta
		float val_2 = (a * a + b * b - c * c) / (2 * a * b); //gamma
		cout << "val_1: " << val_1 << '\n';
		cout << "val_2: " << val_2 << '\n';
		float f_ang1 = acos(val_1) * 180.0 / PI;
		float f_ang2 = acos(val_2) * 180.0 / PI;
		if (y >= 0.) {
			if (z >= 0.5) {
				f_ang1 = (90. - f_ang1 - alpha); //ziehe winkel von c ab
				f_ang2 = (180. - f_ang2);
			} else {
				f_ang1 = (180. - f_ang1 - alpha); //ziehe winkel von c ab
				f_ang2 = (180. - f_ang2);

			}
		} else {
			if (z >= 0.5) {
				f_ang1 = (-90. + f_ang1 + (180-alpha)); //ziehe winkel von c ab
				f_ang2 = (- 180. + f_ang2);
			} else {
				f_ang2 = (-180. + f_ang2);
				f_ang1 = +f_ang1 - alpha;
			}
		}
		cout << "f_ang1: " << f_ang1 << '\n';
		cout << "f_ang2: " << f_ang2 << '\n';
		angle1 = f_ang1 * degrees; //beta
		angle2 = f_ang2 * degrees; //gamma
		return true;
	}
};

class ArmTestCmdLineCallback: public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled(
				"ArmMotionTest", true);

		// run managers and wait for termination - comment this line if you want to quit right away
		services.runManagers();

		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion, ArmMotionTest, false, "Test module from arm.cpp")

namespace {
auto cmdTest =
		CommandLine::getInstance().registerCommand<ArmTestCmdLineCallback>(
				"arm", "Robot Arm Test",
				ModuleManagers::none()->enable<Motion>());
}
