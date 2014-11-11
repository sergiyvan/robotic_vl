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

//(inverse) kinematics
#include "tools/kinematicEngine/tasks/kinematicEngineTaskLocation.h"

//io
#include <iostream>
#include <fstream>

/*------------------------------------------------------------------------------------------------*/
BEGIN_DECLARE_MODULE(JacobiArmMotionTest)
	REQUIRE(MotorAngles)
	REQUIRE(KinematicTree)
	PROVIDE(MotorPositionRequest)
END_DECLARE_MODULE(JacobiArmMotionTest)

class JacobiArmMotionTest : public JacobiArmMotionTestBase {
private:
	std::ofstream myfile;
	int state;
	arma::colvec3 goals[4];

	RobotDescription const& robotDescription = *services.getRobotModel().getRobotDescription();;
    MotorID idRoot;
    MotorID id0;
    MotorID id1;
    MotorID id2;
    MotorID idHand;

public:
	JacobiArmMotionTest() {
	}

	virtual void init() {
		myfile.open("values.csv");

		state=0;
        goals[0] << 0. << 0.2 << 0.7;
        goals[1] << 0. << -0.3 << 0.7;
        goals[2] << 0. << -0.2 << 0.3;
        goals[3] << 0. << 0.3 << 0.2;

        idRoot = robotDescription.getEffectorID("root");
        id0 = robotDescription.getEffectorID("motor0");
        id1 = robotDescription.getEffectorID("motor1");
        id2 = robotDescription.getEffectorID("motor2");
        idHand = robotDescription.getEffectorID("hand");
	}

	virtual void execute() {

        arma::colvec3 curLoc;
        arma::mat44 transMatRootToHand = getKinematicTree().getTransitionMatrixFromTo(idRoot, idHand);
        curLoc << transMatRootToHand(0, 3) << transMatRootToHand(1, 3) << transMatRootToHand(2, 3);

        //check if reached goal
		float distToGoal = sqrt(pow(goals[state](1)-curLoc(1), 2)+pow(goals[state](2)-curLoc(2), 2));
		INFO("dist to goal: %f", distToGoal);
        if (distToGoal < 0.000001) {
            INFO("Reached Goal %d", state)
            state = (state + 1) % 4;
        }

        Degree angle0 = 0*degrees;
        Degree angle1 = 0*degrees;
        Degree angle2 = 0*degrees;

//		anglesFromJacobian(angle0, angle1, angle2, curLoc, curTargetLoc);

        Degree curAngle0 = getMotorAngles().getPosition(id0);
        Degree curAngle1 = getMotorAngles().getPosition(id1);
        Degree curAngle2 = getMotorAngles().getPosition(id2);

        getMotorPositionRequest().setPositionAndSpeed(id1, curAngle0+angle0, 80. * rounds_per_minute);
        getMotorPositionRequest().setPositionAndSpeed(id1, curAngle1+angle1, 80. * rounds_per_minute);
        getMotorPositionRequest().setPositionAndSpeed(id2, curAngle2+angle2, 80. * rounds_per_minute);

        //output on console
        INFO("TARGET  x: %f y: %f z: %f", goals[state](0), goals[state](1), goals[state](2));
        INFO("CURRENT x: %f y: %f z: %f", curLoc(0), curLoc(1), curLoc(2));

        //output in file
		myfile << transMatRootToHand(0, 3) << ","
				<< transMatRootToHand(1, 3) << ","
				<< transMatRootToHand(2, 3) << ","
				<< std::endl;
	}

	void anglesFromJacobian(Degree& angle0, Degree& angle1, Degree& angle2, arma::colvec3 curLocation, arma::colvec3 targetLocation) {

		//the KinematicEngineTaskLocation needs a target to calculate a valid jacobian (which is independent from the target). Ask Lutz why!
        KinematicEngineTaskLocation locTask = KinematicEngineTaskLocation("locTask", idRoot, idHand, getKinematicTree(), goals[state]);
        arma::mat jacobian = locTask.getJacobianForTask(getKinematicTree());
        arma::mat pseudoInverseJacobian = buildPseudoInverse(jacobian, 0.001);

        //insert your code here!

        //output on console
//        INFO("Jacobian (%dx%d)", jacobian.n_rows, jacobian.n_cols);
//		INFO("\n(%f, %f, %f)\n(%f, %f, %f)\n(%f, %f, %f)",
//				jacobian(0,0), jacobian(0,1), jacobian(0,2),
//				jacobian(1,0), jacobian(1,1), jacobian(1,2),
//				jacobian(2,0), jacobian(2,1), jacobian(2,2)
//		);
//        INFO("pseudoInverseJacobian (%dx%d)", pseudoInverseJacobian.n_rows, pseudoInverseJacobian.n_cols);
//		INFO("\n(%f, %f, %f)\n(%f, %f, %f)\n(%f, %f, %f)",
//				pseudoInverseJacobian(0,0), pseudoInverseJacobian(0,1), pseudoInverseJacobian(0,2),
//				pseudoInverseJacobian(1,0), pseudoInverseJacobian(1,1), pseudoInverseJacobian(1,2),
//				pseudoInverseJacobian(2,0), pseudoInverseJacobian(2,1), pseudoInverseJacobian(2,2)
//		);

	}


	arma::mat buildPseudoInverse(arma::mat matrix, double epsilon) const
	{
		arma::mat helper = matrix * matrix.t();
		const arma::mat I = arma::eye(helper.n_rows, helper.n_cols);
		arma::mat pseudoInverseJacobian = matrix.t() * arma::inv((helper + epsilon * I));
		return pseudoInverseJacobian;
	}

};



class JacobiArmTestCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled("JacobiArmMotionTest", true);

		// run managers and wait for termination - comment this line if you want to quit right away
		services.runManagers();

		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion,    JacobiArmMotionTest,    false, "Test module from jacobi.cpp")

namespace {
	auto cmdTest = CommandLine::getInstance().registerCommand<JacobiArmTestCmdLineCallback>(
			"jacobi",
			"Robot JacobiArm Test",
			ModuleManagers::none()->enable<Motion>() );
}
