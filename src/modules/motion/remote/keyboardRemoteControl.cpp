#include "keyboardRemoteControl.h"

#include "platform/hardware/robot/robotDescription.h"

#include "management/commandLine.h"

#include "modules/motion/motion.h"

#include "modules/motion/walking/motion_walker_base.h"

#include "utils/keyboard.h"

#define OUTPUT_INTERVAL (3000*milliseconds)
#define INCREMENT_VALUE    10


/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion, KeyboardRemoteControl, false, "Module allowing manual (keyboard) control of the robot")

/*------------------------------------------------------------------------------------------------*/

class WalkerCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		takeKeyboard();
		services.getModuleManagers().get<Motion>()->setModuleEnabled("KeyboardRemoteControl", true);
		services.runManagers();
		releaseKeyboard();

		return true;
	}
};

namespace {
	auto cmdWalker = CommandLine::getInstance().registerCommand<WalkerCmdLineCallback>(
			"walker",
			"Manually controlled walking (without cognition)",
			ModuleManagers::none()->enable<Motion>(2));
	auto cmdWalkerCognition = CommandLine::getInstance().registerCommand<WalkerCmdLineCallback>(
			"walker_with_cognition",
			"Manually controlled walking (with cognition)",
			ModuleManagers::all()->enable<Motion>(2));
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

KeyboardRemoteControl::KeyboardRemoteControl()
	: lastSpeedInformation(0)
	, disableMotors(0)
	, allowHeadMovements(false)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

KeyboardRemoteControl::~KeyboardRemoteControl() {

}


/*------------------------------------------------------------------------------------------------*/

/** Initialize the module. This is called by the framework when execute() is
 ** called for the first time.
 */

void KeyboardRemoteControl::init() {
	printInstructions();

	allowHeadMovements = false;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void KeyboardRemoteControl::execute() {
	getMotionStatus().cognitionInputEnabled = false;
	getMotionStatus().allowHeadMovements = allowHeadMovements;

	bool printWalkerSpeed = false;

	Degree headPitch = Degree(getMotorAngles().getPosition(MOTOR_HEAD_PITCH));
	Degree headYaw   = Degree(getMotorAngles().getPosition(MOTOR_HEAD_TURN));

	// controls for the robot
	int c = getKeyWithUsTimeout(0);
	if (lastSpeedInformation + OUTPUT_INTERVAL < getCurrentTime())
		printWalkerSpeed = true;
	else if (c <= 0)
		return;

	// handle keyboard input
	switch (c) {
	case '?':
		printInstructions();
		return;

	// -----------------------
	// WALKER
	// stop everything
	case 'x':
		INFO("Stopping all motions ...");
		getMotionRequest().motion = MOTION_NONE;
		break;

	// Initiate Walking
	case 'v':
		INFO("Starting walker with no speed ...");
		getMotionRequest().motion = MOTION_LOCOMOTION;
		getWalkerTargetValues().setTargetSpeeds({0.0, 0.0, 0.0});
		break;

	// Faster
	case 'w':
		printWalkerSpeed = true;

		printf("BEFORE Target: %f, Current: %f\n", getWalkerTargetValues().getTargetSpeeds().x, getMotionRequest().walkerParam.getForwardSpeed());
		getWalkerTargetValues().setTargetSpeedX(getMotionRequest().walkerParam.getForwardSpeed() + INCREMENT_VALUE);
		printf("AFTER Target: %f, Current: %f\n", getWalkerTargetValues().getTargetSpeeds().x, getMotionRequest().walkerParam.getForwardSpeed());

		break;

	// Slower
	case 's':
		printWalkerSpeed = true;

		getWalkerTargetValues().setTargetSpeedX(getMotionRequest().walkerParam.getForwardSpeed() - INCREMENT_VALUE);

		break;

	// strafe left
	case 'a':
		printWalkerSpeed = true;

		getWalkerTargetValues().setTargetSpeedY(getMotionRequest().walkerParam.getSidewardSpeed() + INCREMENT_VALUE);

		break;

	// strafe right
	case 'd':

		printWalkerSpeed = true;

		getWalkerTargetValues().setTargetSpeedY(getMotionRequest().walkerParam.getSidewardSpeed() - INCREMENT_VALUE);

		break;

	// Rotate left
	case 'e':

		printWalkerSpeed = true;

		getWalkerTargetValues().setTargetSpeedYaw(getMotionRequest().walkerParam.getRotationSpeed() - INCREMENT_VALUE);

		break;

	// Rotate right
	case 'q':

		printWalkerSpeed = true;

		getWalkerTargetValues().setTargetSpeedYaw(getMotionRequest().walkerParam.getRotationSpeed() + INCREMENT_VALUE);


		break;

	// left foot drift
	case 'y':
		getMotionRequest().walkerParam.setFootDrift(getMotionRequest().walkerParam.getFootDrift() + 3);
		printWalkerSpeed = true;
		break;

	// right foot drift
	case 'c':
		getMotionRequest().walkerParam.setFootDrift(getMotionRequest().walkerParam.getFootDrift() - 3);
		printWalkerSpeed = true;
		break;

	// stand up
	case 'A':
		INFO("Standup requested");
		getMotionRequest().motion = MOTION_STANDUP;
		break;

	// Idle
	case 'i':
		INFO("Idle requested");

		getWalkerTargetValues().setTargetSpeeds({0.0, 0.0, 0.0});
		getMotionRequest().motion = MOTION_STAND_IDLE;

		break;

	// -----------------------
	// HEAD
	case 'z':
		if (headYaw > -135*degrees) {
			getMotorPositionRequest().setPosition(MOTOR_HEAD_TURN, headYaw - 3*degrees);
			getMotorPositionRequest().setSpeed(MOTOR_HEAD_TURN,    28*rounds_per_minute);
		}
		break;

	case 't':
		if (headYaw < 135*degrees) {
			getMotorPositionRequest().setPosition(MOTOR_HEAD_TURN, headYaw + 3*degrees);
			getMotorPositionRequest().setSpeed(MOTOR_HEAD_TURN,    28*rounds_per_minute);
		}
		break;

	case 'u':
		if (headPitch > -90*degrees) {
			getMotorPositionRequest().setPosition(MOTOR_HEAD_PITCH, headPitch - 3*degrees);
			getMotorPositionRequest().setSpeed(MOTOR_HEAD_PITCH,    28*rounds_per_minute);
		}
		break;

	case 'j':
		if (headPitch < 90*degrees) {
			getMotorPositionRequest().setPosition(MOTOR_HEAD_PITCH, headPitch + 3*degrees);
			getMotorPositionRequest().setSpeed(MOTOR_HEAD_PITCH,    28*rounds_per_minute);
		}
		break;

	// -----------------------
	// KICKER
	case 'g':
		MotionWalker::kickLeft();
		break;

	case 'h':
		MotionWalker::kickRight();
		break;

	case 'b':
		getMotionRequest().motion = MOTION_KICK;
		getMotionRequest().kickParam.leg = KICK_WITH_LEFT;
		break;

	case 'n':
		getMotionRequest().motion = MOTION_KICK;
		getMotionRequest().kickParam.leg = KICK_WITH_RIGHT;
		break;

	// -----------------------
	// RANDOM

	// disable all motors
	case 'O':
		disableMotors = 2;
		break;

	case 'F':
		if (disableMotors)
			getMotorPositionRequest().setAllTorques(false);
		break;

	// toggle gaze
	case 'p': {
		allowHeadMovements = !allowHeadMovements;
		break;
	}

	case '9': {
		INFO("Wave Motion...");

		static bool left = false;
		if (true == left)
		{
			getMotionRequest().motion = MOTION_WAVE_LEFT;
		} else
		{
			getMotionRequest().motion = MOTION_WAVE_RIGHT;
		}
		left = !left;
		break;
	}

	case '7': {
		INFO("Chicken dance Motion...");
		getMotionRequest().motion = MOTION_CHICKEN_DANCE;
		break;
	}

	case '6': {
		INFO("Hip dance Motion...");
		getMotionRequest().motion = MOTION_HIP_DANCE;
		break;
	}

	case '5': {
		INFO("Cheer Motion...");
		getMotionRequest().motion = MOTION_CHEER;
		break;
	}

	default:
		break;
	}

	if (disableMotors)
		disableMotors--;

	// if we got here, some action is required so do some preparations

	if (printWalkerSpeed) {
		if (getMotionRequest().motion == MOTION_LOCOMOTION) {
			INFO("Walker speed: Forward %f, Sideways %f, Rotation %f ... Foot drift %f",
				getMotionRequest().walkerParam.getForwardSpeed(),
				getMotionRequest().walkerParam.getSidewardSpeed(),
				getMotionRequest().walkerParam.getRotationSpeed(),
				getMotionRequest().walkerParam.getFootDrift());
		}
		lastSpeedInformation = getCurrentTime();
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void KeyboardRemoteControl::printInstructions() {
	printf("==============================\n|   I N S T R U C T I O N S  |\n==============================\n\n");
	printf("WALKER\n");
	printf(" v - start walker\n");
	printf(" x - stop walker\n");
	printf(" w - forward\n");
	printf(" s - backward\n");
	printf(" a - sidestep left\n");
	printf(" d - sidestep right\n");
	printf(" q - rotation left\n");
	printf(" e - rotation right\n");
	printf(" y - left foot drift\n");
	printf(" c - right foot drift\n");

//	printf("\nKICKER\n");
//	printf(" b - kick left\n");
//	printf(" n - kick right\n");
//	printf(" g - mini kick left\n");
//	printf(" h - mini kick right\n");
//
//	printf("\nHEAD\n");
//	printf(" p - toggle between gaze and manual head movement\n");
//	printf(" t - turn head left\n");
//	printf(" z - turn head right\n");
//	printf(" u - move head up\n");
//	printf(" j - move head down\n");
//
//	printf("\nENTERTAINMENT\n");
//	printf(" 0 - ByeBye\n");
//	printf(" 9 - Wave\n");
//	printf(" 8 - Clapping Hands\n");
//	printf(" 7 - Chickendance\n");
//	printf(" 6 - Hipdance\n");
//	printf(" 5 - Cheer\n");
//	printf(" 1 - Sit down\n");
//
//	printf("\nMOTIONS\n");
//	printf(" A - standup");
//	printf(" i - idle\n");

	printf("\nOTHER\n");
	printf(" O - disable motors (followed by F)\n");
	printf("\n Ctrl-C - EXIT\n");
	printf(" ? - print this help\n");

	printf("\n\n");
}
