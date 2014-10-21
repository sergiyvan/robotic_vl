#include "learningWalker.h"

#include <math.h>

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"
#include "modules/motion/kinematic/kinematic2013.h"

#include "debug.h"
#include "services.h"
#include "management/config/configRegistry.h"
#include "management/config/config.h"

#include "utils/math/Common.h"
#include "utils/math/Math.h"

#include <stdio.h>
#include <iostream>

#include <fstream>

/*------------------------------------------------------------------------------------------------*/

LearningWalker::Side operator!(const LearningWalker::Side& s) {
	if (s == LearningWalker::Side::LEFT) return LearningWalker::Side::RIGHT;
	return LearningWalker::Side::LEFT;
}

/*------------------------------------------------------------------------------------------------*/

REGISTER_DEBUG("motions.learningwalker.pos",   PLOTTER, BASIC);
REGISTER_DEBUG("motions.learningwalker.footX", PLOTTER, BASIC);
REGISTER_DEBUG("motions.learningwalker.footY", PLOTTER, BASIC);
REGISTER_DEBUG("motions.learningwalker.footZ", PLOTTER, BASIC);
REGISTER_DEBUG("motions.learningwalker.timer", PLOTTER, BASIC);

REGISTER_DEBUG("motions.learningwalker.load", PLOTTER, BASIC);


REGISTER_DEBUG("motions.learningwalker.gyro", PLOTTER, BASIC);

REGISTER_DEBUG("motions.learningwalker.odometry.control", TABLE, BASIC);
REGISTER_DEBUG("motions.learningwalker.odometry.values", TABLE, BASIC);

/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgSection = ConfigRegistry::getSection("motions.learningwalker");

	auto cfgStepsPerSecond   = cfgSection->registerOption<double>("stepsPerSecond",          3.75f, "The frequency of the walker.");
	auto cfgStartingTime     = cfgSection->registerOption<int>("startingTime",            2000, "The duration of the starting sequence.");

	auto cfgLegLength        = cfgSection->registerOption<int>("leglength",                310, "Length of the maximal stretched leg during walk (! - could be shorter than the real leg)");
	auto cfgOrthoStepWidth   = cfgSection->registerOption<int>("orthostepwidth",            31, "The width between the legs during walk the ortho dribbling.");
	auto cfgStepHeight       = cfgSection->registerOption<int>("stepheight",                25, "The height the leg is lifted when walking.");

	// non ik movements
	auto cfgShoulderMovement = cfgSection->registerOption<double>("shouldermovement",        1.5f, "Increase oder decrease arm movement");
	auto cfgFootRoll         = cfgSection->registerOption<Degree>("footroll",                3*degrees,  "Bend foot to the inner side for stability reasons");

	// pendulum
	auto cfgPendulumAmp      = cfgSection->registerOption<Degree>("pendulumAmplitude",     3.0*degrees, "In degrees: How much the spine motor is rotating");
	auto cfgInnerYOffset     = cfgSection->registerOption<double>("innerYOffset",           -0.0f, "Lateral offset of end effector. Negative -> outside, positive -> inside");
	auto cfgXOffset          = cfgSection->registerOption<Millimeter>("xOffset",           5*millimeters, "positive->forward, negative-> backward (in mm)");
	auto cfgYawOffset        = cfgSection->registerOption<Millimeter>("yawOffset",         0*millimeters, "positive->forward, negative-> backward (in mm)");

	// Learning
	auto cfgMagicLearn       = cfgSection->registerOption<bool>("magicLearn",                 false, "1: Robot loads, saves and learns.");

	auto cfgLearn            = cfgSection->registerOption<bool>("learn",                      false, "1: Robot learns from new experience. 0: It does not.");
	auto cfgShowStateInfo    = cfgSection->registerOption<bool>("showStateInformation",       false, "1: Displays information about the current learning state. 0: It does not.");
	auto cfgLoadExperience   = cfgSection->registerOption<bool>("loadExperience",             false, "1: Robot loads previously learned experience at the beginning.; 0: It does not");
	auto cfgSaveExperience   = cfgSection->registerOption<bool>("saveExperience",             false, "1: Robot saves new experiences.; 0: It does not");
	auto cfgExplorationProb  = cfgSection->registerOption<double>("explorationProbability",  0.1f, "Probability that the robot chooses a random action");
	auto cfgDiscountFactor   = cfgSection->registerOption<double>("discountFactor",          0.9f, "Factor to change importance of reinforcement at distant time steps");
	auto cfgParamFile        = cfgSection->registerOption<std::string>("paramfile", "config/walkerparams.pbw", "File name to load/save learned walking parameters");
	auto cfgLearningRate     = cfgSection->registerOption<double>("learningRate",           0.02f, "Learning Rate: 0: only use old information. 1: only use new information");
	auto cfgMotorSpeeds      = cfgSection->registerOption<double>("motorSpeeds",             114.f, "speeds the motors turn [0., 114.]");
	auto cfgPendulumFactorY  = cfgSection->registerOption<double>("pendulumFactorY",         0.5f, "multiplied to lateral pendulum movement");
	auto cfgSpineRollFactor  = cfgSection->registerOption<double>("spineRollFactor",         0.0f, "multiplied to spine movement");

	auto cfgStomachOffset    = cfgSection->registerOption<double>("stomachOffset",           4.0f, "added to stomach pitch motor");
	auto cfgStomachFactor    = cfgSection->registerOption<double>("stomachFactor",           1.5f, "multiplied stomach pitch motor according to speedX");
	auto cfgMaxGyro          = cfgSection->registerOption<double>("maxGyro",                 0.0f, "slow down motion if gyro measures greater than this value");
	auto cfgSlowDownFactor   = cfgSection->registerOption<double>("slowDownFactor",          0.0133f, "factor multiplied with stepspersecond if gyro measures greater than maxGyro");
	auto cfgStabilityYFactor = cfgSection->registerOption<double>("stabilityYFactor",        0.0f, "factor multiplied with stepspersecond if gyro measures greater than maxGyro");

	auto cfgInstableAngle    = cfgSection->registerOption<double>("instableAngle",           7.0f, "angle at which point the robot refuses to perform minikick");
	auto cfgTimeToStabilize  = cfgSection->registerOption<Millisecond>("timeToStabilize", 1000*milliseconds, "time the walker needs before it can perform minikick after being unstable");

	auto cfgMaxFootDrift     = cfgSection->registerOption<double>("maxFootDrift",           50.0f, "max distance between legs along x axis while orthodribbling");
	auto cfgSuppressionZ     = cfgSection->registerOption<double>("suppressionZ",            0.0f, "extra lift along z in shift phase that is corrected during support phase");
	auto cfgXFactor          = cfgSection->registerOption<double>("xFactor",                 0.3f, "multiplied with speed x and added to xoffset");
}


/*------------------------------------------------------------------------------------------------*/



LearningWalker::LearningWalker() :
	ik(new Kinematic2013()),
	starter(MW_Starter(0)),
	startKick(false),
	useReinforcementLearning(false),
	smoothedXSpeed(0),
	smoothedYSpeed(0),
	stopWalkingFlag(false),
	xpCount(0),
	quitWalkerFlag(false),
	maxGyro(0.0),
	speedsWereSet(false),
	timeLastNotStable(0.0 * milliseconds)
{
}


/*------------------------------------------------------------------------------------------------*/


LearningWalker::~LearningWalker() {
	services.getEvents().unregisterForEvent(EVT_CONFIGURATION_LOADED, this);
	delete ik;
}


/*------------------------------------------------------------------------------------------------*/



void LearningWalker::init() {
	starter = MW_Starter( cfgStartingTime->get());

	timer[Side::LEFT].timer.setCyclic(true);
	timer[Side::RIGHT].timer.setCyclic(true);
	speed[Side::LEFT] = {0, 0, 0};
	speed[Side::RIGHT] = {0, 0, 0};
	learnData[Side::LEFT] = LearnData();
	learnData[Side::RIGHT] = LearnData();

	idle = services.getRobotModel().getIdleValues();

	// Register for configuration changes
	services.getEvents().registerForEvent(EVT_CONFIGURATION_LOADED, this);
	eventCallback(EVT_CONFIGURATION_LOADED, &services.getConfig());

	// Set steps per second
	for (auto& t : timer) {
		t.second.timer.setStepsPerSecond(stepsPerSecond);
	}
	lastImpact[Side::LEFT].setStepsPerSecond(stepsPerSecond);
	lastImpact[Side::RIGHT].setStepsPerSecond(stepsPerSecond);


	// Set times for walking-cycle
	timeToStabilizeOnOtherFoot = 45; // in 45:  pendulum has half of maximal kinetic energy
	timeToLiftFoot = 45;             // in 90:  pendulum has no kinetic energy

	// initialize learning subsystem if required
	if (useReinforcementLearning && false == neuralLearning.isInitialized())
		initLearning();
}


/*------------------------------------------------------------------------------------------------*/


void LearningWalker::initLearning() {
	RL_Model* model = RL_ModelFactory::createSimpleModel();
	neuralLearning.init(model, discountFactor, 0.8, learningRate, 10);

	for (auto& e : learnData) {
		e.second.learnState    = neuralLearning.model->getDefaultState();
		e.second.action        = neuralLearning.model->getDefaultAction();
		e.second.oldLearnState = e.second.learnState;
		e.second.oldAction     = e.second.action;
	}

	// Initialize Reinforcement Learning Stuff
	if (loadExperience) {
		neuralLearning.loadWhatWasLearnedAsync(parameterFileName);
	}
}


/*------------------------------------------------------------------------------------------------*/


void LearningWalker::execute() {
	// start walking if told so
	if (getActiveMotion().motionInit) {
		startWalking();
	}

	// bail out if requested or if we are not ready yet
	if (quitWalkerFlag || true == neuralLearning.isLoading.load())
		return;

	// Assume we are unstable, we will set it to true below at an appropriate time
	getMotionStatus().robotIsStable = false;

	// set kick params
	if (getMotionRequest().walkerParam.getKickSide() != NO_KICK) {
		if (getMotionRequest().walkerParam.getKickSide() == KICK_WITH_LEFT) {
			kick = LEFT_FOOT;
			kickLeft();
		} else if (getMotionRequest().walkerParam.getKickSide() == KICK_WITH_RIGHT) {
			kick = RIGHT_FOOT;
			kickRight();
		}
	}

	computeSpeeds(Side::LEFT);
	computeSpeeds(Side::RIGHT);

	handleInstability();


	// synchronize steps
	stepSync(Side::LEFT);
	stepSync(Side::RIGHT);

	// perform last step until its end (i.e. time >= 360)
	if (stopWalkingFlag) {
		if (std::abs(smoothedXSpeed) + std::abs(smoothedYSpeed) < 3) {
			getMotionStatus().robotIsStable = true;
			getMotionStatus().motionHasFinished = true;
			quitWalkerFlag = true;
			for (auto& e : idle) {
				getMotorPositionRequest().setPosition(e.first, e.second);
			}
			return;
		}
	}


	// here we use the trajectories
	computeFootPoses();


	// stop walking if told so
	if (getActiveMotion().stopWhenStable) {
		stopWalkingFlag = true;
	}

	// Update SupportFoot
	if (getTime(Side::LEFT) < 180) {
		getSupportFoot().setSupportFoot(SupportFoot::LEFT);
	} else {
		getSupportFoot().setSupportFoot(SupportFoot::RIGHT);
	}

	// Plot stuff
	//double stuffToPlot = fabs(gyroRoll.value()) * fabs(gyroRoll.value()) + fabs(gyroPitch.value()) * fabs(gyroPitch.value());

	//double stuffToPlot = fabs(getGyroData().getRoll().value());// * fabs(gyroRoll.value());
	/*double stuffToPlot = right_leg_pose.y;
	std::ofstream myStream;
	myStream.open("ypos.plt", std::ios::out | std::ios::app);
	myStream << std::setprecision(10);
	myStream << stuffToPlot << ", " << left_leg_pose.y;
	myStream << std::endl;
	myStream.close();*/

}


/*------------------------------------------------------------------------------------------------*/


void LearningWalker::computeFootPoses() {
	trajectory.startingFactor = starter.getFactor();

	if (getTime(Side::LEFT) <= 360) {

		// compute feet positions
		left_leg_pose  = trajectory.getPose(LEFT_FOOT, getTime(Side::LEFT), learnData[Side::LEFT].oldAction, learnData[Side::LEFT].action);
		right_leg_pose = trajectory.getPose(RIGHT_FOOT, getTime(Side::LEFT), learnData[Side::RIGHT].oldAction, learnData[Side::RIGHT].action);

		// Orthodribble
		double footDrift = Math::limited(getMotionRequest().walkerParam.getFootDrift(), -maxFootDrift, maxFootDrift);
		left_leg_pose.x  += footDrift;
		right_leg_pose.x -= footDrift;
		//left_leg_pose.y  -= orthoStepWidth;
		//right_leg_pose.y += orthoStepWidth;

		// compute inverse kinematic
		auto goal = ik->setEndEffectors(left_leg_pose, right_leg_pose);

		// we some motor movements without an inverse kinematic (e.g. arms)
		goal = nonIKMovements(goal, {getTime(Side::LEFT), getTime(Side::RIGHT)});

		// move motors to positions
		for (auto e : goal) {
			getMotorPositionRequest().setPositionAndSpeed(e.first, e.second, motorSpeeds);
		}

		//!TODO obvious wrong place to calculate leanOffset, not needed here
		//!TODO keine Ahnung was hier berechnet wird (SGG)
		const  double angleResolution = 195.5;
		leanOffsetGyroPitch = ((computeStomachPitch().value() /300. * 1024. + 512)/ angleResolution) / Math::pi_180 * degrees;

		// Update Learning
		if (useReinforcementLearning) updateLearning();
	}
}

/*------------------------------------------------------------------------------------------------*/

 std::map<MotorID, Degree> LearningWalker::nonIKMovements(std::map<MotorID, Degree> retGoal, std::pair<int, int> timer) const {

	// Move arms forward/backward while walking (MotorId 3+4)
	if (false == getMotionRequest().walkerParam.getNoArmMovements()) {
		retGoal[MOTOR_LEFT_ARM_PITCH]  = computeShoulder(Side::LEFT, timer.first);
		retGoal[MOTOR_RIGHT_ARM_PITCH] = computeShoulder(Side::RIGHT, timer.second);
	}

	// Roll-Stabilization performed by arm movement (MotorId 5+6)
	retGoal[MOTOR_LEFT_ARM_ROLL]   = computeStabilizationArmRoll(Side::LEFT);
	retGoal[MOTOR_RIGHT_ARM_ROLL]  = computeStabilizationArmRoll(Side::RIGHT);

	// Hold hands up (they break if hold straight...) (MotorID 7+8)
	retGoal[MOTOR_LEFT_ELBOW]  = 130*degrees;
	retGoal[MOTOR_RIGHT_ELBOW] = -130*degrees;

	// (MotorID 9)
	retGoal[MOTOR_STOMACH] = computeStomachPitch();

	// Spine is keeping upper body upright (MotorID 10)
	retGoal[MOTOR_SPINE] = computeSpineRoll(timer.first, timer.second);

	// Passive Dynamic Walking and Roll-Stabilization performed by feet movement (MotorId 19+20)
	retGoal[MOTOR_LEFT_FOOT_ROLL]  += computeFootRoll(Side::LEFT);
	retGoal[MOTOR_RIGHT_FOOT_ROLL] += computeFootRoll(Side::RIGHT);

	return retGoal;
}



/*------------------------------------------------------------------------------------------------*/



void LearningWalker::handleInstability() {

	//float angle = std::max(std::abs(getGyroData().getRoll().value()), std::abs(getGyroData().getPitch().value()));
	float angle = getGyroData().getRoll().value();
	if (angle > instableAngle) {
		timeLastNotStable = getCurrentTime();
	}


	double fraction =  std::abs(getGyroData().getRoll().value()) * slowDownFactor;
	fraction = Math::limited(1 - fraction, 0.1, 1.0);

//	if (std::abs(getGyroData().getRoll().value()) > maxGyro) {

		timer[Side::LEFT].timer.setStepsPerSecond(stepsPerSecond * fraction);
		timer[Side::RIGHT].timer.setStepsPerSecond(stepsPerSecond * fraction);
//	}

	if (std::abs(getGyroData().getRoll().value()) > maxGyro) {
		trajectory.innerYOffset = innerYOffset + std::abs(getGyroData().getRoll().value()) * stabilityYFactor;
	}
}



/*------------------------------------------------------------------------------------------------*/


void LearningWalker::handleFallingDown() {
	// The robot posture is set in the postureModule and depends on gyroData.
//	if (getRobotPosture().posture == ROBOT_STANDING) {
//		return;
//	}

	// Reset Learning
	if (useReinforcementLearning) {
		for (auto& e : learnData) {
			e.second.learnState    = neuralLearning.model->getDefaultState();
			e.second.action        = neuralLearning.model->getDefaultAction();
			e.second.oldLearnState = e.second.learnState;
			e.second.oldAction     = e.second.action;
		}
	}


	// If we fell down during miniKick, then we have to reset the flag manually
	startKick = false;

	// We quit now, somebody else has to get us back up
	getMotionStatus().motionHasFinished = true;
	getMotionStatus().robotIsStable     = false;
	quitWalkerFlag = true;
}


/*------------------------------------------------------------------------------------------------*/


void LearningWalker::computeSpeeds(Side side) {

	// if in starting/stopping phase: set all speeds to zero
	double smoothedYawSpeed(0.0);

	if (starter.getFactor() < 1) {
		smoothedXSpeed   = 0.0;
		smoothedYSpeed   = 0.0;
	} else {
		smoothedXSpeed   = getMotionRequest().walkerParam.getForwardSpeed();
		smoothedYSpeed   = getMotionRequest().walkerParam.getSidewardSpeed();
		smoothedYawSpeed = getMotionRequest().walkerParam.getRotationSpeed();
	}

	// Only change speeds if robot is not in the middle of a step
	if ( ((getTime(side) > 0 && getTime(side) < 10)   ||   (getTime(side) > 180 && getTime(side) < 190)) && !speedsWereSet) {
		speedsWereSet = true;

		if (startKick && !speedForMiniKickSet) {
			//smoothedXSpeed += min(smoothedXSpeed + 30.0, smoothedYSpeed); //min(smoothedXSpeed + 50.0, smoothedYSpeed);
			smoothedXSpeed = 300;
			speedForMiniKickSet = true;

		} else if (!startKick) {
			speedForMiniKickSet = false;
		}
		speed[side].x = smoothedXSpeed;
		speed[side].y = smoothedYSpeed;
		speed[side].yaw = smoothedYawSpeed;

		// Tell the trajectory about the new speeds

		float trajectorySpeedX = 0.0;
		if (smoothedXSpeed > 0) {
			trajectorySpeedX = (smoothedXSpeed / 100.0) * float(maxForwardSpeed);
		} else {
			trajectorySpeedX = (smoothedXSpeed / 100.0) * float(maxBackwardSpeed);
		}

		float trajectorySpeedY   = (smoothedYSpeed / 100.0) * float(maxSidewardSpeed);
		float trajectorySpeedYaw = (smoothedYawSpeed / 100.0) * float(maxRotationSpeed);

		if (!startKick) {
			smoothedXSpeed = trajectorySpeedX;
			smoothedYSpeed = trajectorySpeedY;
			smoothedYawSpeed = trajectorySpeedYaw;

			speed[side].x = smoothedXSpeed;
			speed[side].y = smoothedYSpeed;
			speed[side].yaw = smoothedYawSpeed;

			trajectory.setSpeeds(trajectorySpeedX, trajectorySpeedY, trajectorySpeedYaw);
			trajectory.stepHeight = stepHeight;

			trajectory.innerYOffset = innerYOffset;

		} else {

			//printf("minikick!!!\n");


			trajectory.currentSpeeds.x = Math::limited(smoothedXSpeed + 15, 0, 25);
			trajectory.currentSpeeds.y = 0.0;
			trajectory.currentSpeeds.yaw = 0.0;


//			speed[side].x = smoothedXSpeed;
//			speed[side].y = smoothedYSpeed;
//			speed[side].yaw = smoothedYawSpeed;

			trajectory.stepHeight = stepHeight + 15;

			trajectory.innerYOffset = innerYOffset + 10;

			trajectory.maxSpeeds.x   = float(maxForwardSpeed);
			trajectory.maxSpeeds.y   = float(maxSidewardSpeed);
			trajectory.maxSpeeds.yaw = float(maxRotationSpeed);

		}


	} else {
		speedsWereSet = false;
	}
}


/*------------------------------------------------------------------------------------------------*/


void LearningWalker::startWalking() {

	starter.start();
	timer[Side::LEFT].timer.startTimer();
	timer[Side::LEFT].timer.setTimer(0);
	timer[Side::RIGHT].timer.startTimer();
	timer[Side::RIGHT].timer.setTimer(180);

	left_leg_pose.x   = right_leg_pose.x   = 0;
	left_leg_pose.y   = right_leg_pose.y   = 0;
	left_leg_pose.z   = right_leg_pose.z   = legLength;
	left_leg_pose.yaw = right_leg_pose.yaw = 0;

	stopWalkingFlag = false;
	quitWalkerFlag = false;

	INFO("startWalking(): Time Left: %d, Time Right: %d", getTime(Side::LEFT), getTime(Side::RIGHT));
}


/*------------------------------------------------------------------------------------------------*/


bool LearningWalker::stepSync(Side side) {

	//TODO: This code is a giant mess!

	bool impact = false;
	if (true /* getGroundContact().isOnGround(side) */ ) {
		// if we have early impacts we ignore them



		if (getTime(side) >= 180) {


			// ignore impact timer while starting, because we lift the
			// feet less and can get more impacts than normal

			if (lastImpact[side].getTime() > 10) {

				// if the non-kick foot starts its swinging phase stop the kick
				if (startKick && ( (kick == LEFT_FOOT && Side::LEFT == side)
				                 ||(kick == RIGHT_FOOT && Side::RIGHT == side))) {
					kick = NO_FOOT;
					startKick = false;
					trajectory.startKick = false;

					// stop to regain stability
					getWalkerTargetValues().setTargetSpeeds({0., 0., 0.});
				}

				// if the kick foot starts its swinging phase start the kick
				if (kick != NO_FOOT && (getCurrentTime() - timeLastNotStable) > timeToStabilize) {
					startKick = true;
					trajectory.startKick = true;
				}


				if (getTime(!side) >= 350) {
					// at this point we have an impact event
					impact = true;

					// plot the impact time
					DEBUG_PLOTTER("motions.learningwalker.pos", "ground_contact", getCurrentTime(), 375);

					// so we want to start the step with the other foot to minimize
					// the double support phase
					timer[!side].timer.startTimer();
				}
			}

			// restart the timer, which holds the time of the last ground contact
			lastImpact[side].startTimer();

		} else if (getTime(side) >= 350 && getActiveMotion().stopWhenStable) {
			// if the walker was requested to stop, honor this request
			// even if we did not have ground contact in a while (chances
			// are that we won't recover at this time anyway)
			//getMotionStatus().robotIsStable = true;
		}
	}

	// true on impact, false in every other phase.


	/*
	 * Sometimes there are laggy moments in which the timers could run out
	 * and are not able anymore to trigger each other. Here we take care
	 * of this edge case.
	 */
	if (getTime(Side::LEFT) >= 360 && getTime(Side::RIGHT) >= 360) {
		timer[Side::LEFT].timer.startTimer();
	}

	return impact;
}


/*------------------------------------------------------------------------------------------------*/


Degree LearningWalker::computeSpineRoll(int currentTimeLeft, int currentTimeRight) const {
	Degree amplitude = pendulumAmplitude * spineRollFactor;

	Degree pendulumAngle = -amplitude * sin(currentTimeRight * Math::pi_180);
	//pendulumAngle -= 1 * speed[Side::LEFT].y;


	if (startKick) {
		Degree intensity = 1.5*degrees;
		if (currentTimeRight > 180 && kick == LEFT_FOOT) {
			pendulumAngle -= intensity;
		} else if (currentTimeLeft > 180 && kick == RIGHT_FOOT) {
			pendulumAngle += intensity;
		}
	}

	// PD Controller
	//double speed = getMotorPositionRequest().getSpeed(MOTOR_SPINE).value();
	//pendulumAngle -= (getGyroData().getRoll().value() * 2 - speed / 50.0);

	return pendulumAngle;
}

/*------------------------------------------------------------------------------------------------*/

Degree LearningWalker::computeStomachPitch() const {

	// Robot leans forward with torso pitch motor so it does not fall on its back
	double stomachAdd = 0;
	if (!startKick) {
		const int    OFFSET_LEAN      = stomachOffset;//5.0;
			if ((speed.at(Side::LEFT).x >= 0)) {
				//stomachAdd = fmin((speed[Side::LEFT].x * 1.0), 30) + OFFSET_LEAN;
				stomachAdd = Math::limited(speed.at(Side::LEFT).x * stomachFactor, 0, maxForwardSpeed * stomachFactor) + OFFSET_LEAN;
			} else {
				stomachAdd = (speed.at(Side::LEFT).x * 1.0) + OFFSET_LEAN;
			}
	} else {
		stomachAdd = Math::limited(speed.at(Side::LEFT).x * -stomachFactor * 0.1, 0, maxForwardSpeed * stomachFactor);
	}
	return stomachAdd/1024.*300.*degrees;
}

/*------------------------------------------------------------------------------------------------*/


Degree LearningWalker::computeStabilizationArmPitch(Side side) const {
	const double factor = 0.44;     // To what extent we perform stabilization, depending on gyroscope
	const Degree keepArmsBack = 17.5*degrees;

	Degree stabilizeArmPitch = getGyroData().getPitch() * factor;

	if (side == Side::LEFT) {
		stabilizeArmPitch = stabilizeArmPitch + keepArmsBack;
	} else {
		stabilizeArmPitch = -stabilizeArmPitch - keepArmsBack;
	}
	return stabilizeArmPitch;
}


/*------------------------------------------------------------------------------------------------*/


Degree LearningWalker::computeStabilizationArmRoll(Side side) const {
	const double factor    = 0.88;             // To what extent we perform stabilization, depending on gyroscope
	const Degree keepArmsOutside = 25*degrees; // Keep arms outside

	Degree stabilizeArmRoll = getGyroData().getRoll() * factor;

	if (side == Side::LEFT) {
		stabilizeArmRoll -= keepArmsOutside;
	} else {
		stabilizeArmRoll += keepArmsOutside;
	}

	return stabilizeArmRoll;
}


/*------------------------------------------------------------------------------------------------*/


Degree LearningWalker::computeShoulder(Side side, int currentTime) const {
	float speed = this->speed.at(side).x;
	const double factor = 3.5;

	float value = Math::limited(speed * factor, 0.0, 60.0);

	Degree retVal;
	if (currentTime < 180) {
		retVal = float((currentTime/180.0) * -value + speed) / 1024.*300.*degrees;
	} else {
		retVal = float(((currentTime-180)/180.0) * value - speed) / 1024.*300*degrees;
	}
	if (side == Side::LEFT) {
		retVal = -retVal;
	}

	return retVal + computeStabilizationArmPitch(side);
}



/*------------------------------------------------------------------------------------------------*/


Degree LearningWalker::computeFootRoll(Side side) const {

	//int currentTime = getTime(side);
	Degree value = footRoll;

	/*

	if (startKick) {
		if (currentTime < timeToStabilizeOnOtherFoot + timeToLiftFoot && kick != side) {
			value += 15;
		}
	}
	*/

	if (side == Side::RIGHT) {
		value *= -1;
	}


	return value;
}



/*------------------------------------------------------------------------------------------------*/


void LearningWalker::eventCallback(EventType eventType, void* data) {
	if (eventType == EVT_CONFIGURATION_LOADED) {
		maxBackwardSpeed         = services.getConfig().get<int>("motions.walker.maxBackwardSpeed");
		maxForwardSpeed          = services.getConfig().get<int>("motions.walker.maxForwardSpeed");
		maxSidewardSpeed         = services.getConfig().get<int>("motions.walker.maxSidewardSpeed");
		maxRotationSpeed         = services.getConfig().get<int>("motions.walker.maxRotationSpeed");
		trajectory.maxBackwardSpeed = maxBackwardSpeed;
		trajectory.maxForwardSpeed  = maxForwardSpeed;
		trajectory.maxSidewardSpeed = maxSidewardSpeed;
		trajectory.maxRotationSpeed = maxRotationSpeed;

		footRoll                 = cfgFootRoll->get();
		trajectory.rollOffset    = footRoll.value()/300.*1024.;

		stomachOffset            = cfgStomachOffset->get();
		stomachFactor            = cfgStomachFactor->get();

		stepHeight               = cfgStepHeight->get();
		trajectory.stepHeight    = stepHeight;
		orthoStepWidth           = cfgOrthoStepWidth->get();
		legLength                = cfgLegLength->get();
		trajectory.legLength     = legLength;

		maxFootDrift              = cfgMaxFootDrift->get();

		// Stability and smothing
		stepsPerSecond             = cfgStepsPerSecond->get();
		motorSpeeds                = cfgMotorSpeeds->get()*rounds_per_minute;

		// Pendulumi
		pendulumAmplitude          = cfgPendulumAmp->get();
		trajectory.pendulumAmplitude = pendulumAmplitude.value();

		spineRollFactor            = cfgSpineRollFactor->get();

		innerYOffset               = cfgInnerYOffset->get();
		trajectory.innerYOffset    = innerYOffset;

		xOffset                    = cfgXOffset->get().value();
		trajectory.xOffset         = xOffset;

		yawOffset                  = cfgYawOffset->get().value();
		trajectory.yawOffset       = yawOffset;

		pendulumFactorY            = cfgPendulumFactorY->get();
		trajectory.pendulumFactorY = pendulumFactorY;

		// Learning
		showStateInformation     = cfgShowStateInfo->get();
		learn                    = cfgLearn->get();
		loadExperience           = cfgLoadExperience->get();
		saveExperience           = cfgSaveExperience->get();
		explorationProbability   = cfgExplorationProb->get();
		discountFactor           = cfgDiscountFactor->get();
		learningRate             = cfgLearningRate->get();

		parameterFileName        = cfgParamFile->get();

		maxGyro                  = cfgMaxGyro->get();
		slowDownFactor           = cfgSlowDownFactor->get();
		stabilityYFactor         = cfgStabilityYFactor->get();

		instableAngle            = cfgInstableAngle->get();
		timeToStabilize          = cfgTimeToStabilize->get();
		suppressionZ             = cfgSuppressionZ->get();
		trajectory.suppressionZ  = suppressionZ;

		trajectory.xFactor       = cfgXFactor->get();


		int magicLearn           = cfgMagicLearn->get();
		if (magicLearn) {
			learn                = 1;
			loadExperience       = 1;
			saveExperience       = 1;
			showStateInformation = 1;
		}


		// The variable 'useReinforcementLearning' is used internally.
		if (learn || loadExperience) useReinforcementLearning = 1;
		else useReinforcementLearning = 0;
	}
}

/*------------------------------------------------------------------------------------------------*/


void LearningWalker::updateLearning() {

	int samplesPerCycle = neuralLearning.model->stateSpace["time"].range;
	int framesBetweenSamples = 360 / samplesPerCycle;

	std::map<Side, int> sampleNbr;
	int time = int(getTime(Side::LEFT));
	sampleNbr[Side::LEFT]  = (time / framesBetweenSamples) % samplesPerCycle;
	sampleNbr[Side::RIGHT] = (((time + 180) % 360) / framesBetweenSamples) % samplesPerCycle;

	if (sampleNbr[Side::LEFT] != lastSampledTime) {
		lastSampledTime = sampleNbr[Side::LEFT];
		xpCount++;

		for (auto& e : learnData) {
			e.second.oldLearnState = e.second.learnState;
			e.second.oldAction     = e.second.action;
			// Observe current time
			e.second.learnState.setParameter("time", sampleNbr[e.first], neuralLearning.model);
		}

		// Observe Gyroscope data for current state
		Degree gyroRoll = getGyroData().getRoll();
		Degree gyroPitch = getGyroData().getPitch() - leanOffsetGyroPitch;

		int stateRoll  = Math::limited(int(gyroRoll.value() / 1.5), -5, 5);
		int statePitch = Math::limited(int(gyroPitch.value() / 1.5), -5, 5);

		learnData[Side::LEFT].learnState.setParameter("gyroRoll", stateRoll, neuralLearning.model);
		learnData[Side::RIGHT].learnState.setParameter("gyroRoll", -stateRoll, neuralLearning.model);
		learnData[Side::LEFT].learnState.setParameter("gyroPitch", statePitch, neuralLearning.model);
		learnData[Side::RIGHT].learnState.setParameter("gyroPitch", statePitch, neuralLearning.model);

		int speedX   = getMotionRequest().walkerParam.getForwardSpeed()  / 20;//(100 / neuralLearning.model->stateSpace["speedX"].range);
		int speedY   = getMotionRequest().walkerParam.getSidewardSpeed() / 20;//(100 / neuralLearning.model->stateSpace["speedY"].range);
		int speedYaw = getMotionRequest().walkerParam.getRotationSpeed() / 20;//(100 / neuralLearning.model->stateSpace["speedYaw"].range);


		for (auto& _e : learnData) {
			auto& e = _e.second;
			// Set (given) speeds
			e.learnState.setParameter("speedX", speedX, neuralLearning.model);
			e.learnState.setParameter("speedY", speedY, neuralLearning.model);
			e.learnState.setParameter("speedYaw", speedYaw, neuralLearning.model);

			// Set old positions
			e.learnState.setParameter("x", e.oldAction.getParameter("x"), neuralLearning.model);
			e.learnState.setParameter("y", e.oldAction.getParameter("y"), neuralLearning.model);

			// Set old other foot positions
			e.learnState.setParameter("otherX",  learnData[!_e.first].oldAction.getParameter("x"), neuralLearning.model);
			e.learnState.setParameter("otherY",  learnData[!_e.first].oldAction.getParameter("y"), neuralLearning.model);
		}

		// Observe Reward for the current state
		//float reward = getReward();
		float reward = rewardSum;
		rewardSum = 0;
		rewardCount = 0;

		// Choose Action for the current state
		RL_Action takenAction, takenActionOtherFoot;
		for (auto& _e : learnData) {
			auto& e = _e.second;
			if (learn) {
				e.action = neuralLearning.getEGreedyActionForState(e.learnState, explorationProbability);
			} else {
				e.action = neuralLearning.getBestActionForState(e.learnState, false);
			}
		}

		// Update Model by sampling
		if (learn) {
			for (auto& _e : learnData) {
				auto& e = _e.second;
				RL_Sample sample = {e.oldLearnState, e.oldAction, reward, e.learnState};
				neuralLearning.updateBySample(sample);
			}
		}

		// Print interesting stuff
		if (showStateInformation && xpCount % 1 == 0) {
			//printf("\ngyroPitch: %f | gyroRoll: %f | deltaX: %d | deltaY: %d | deltaZ: %d\n", gyroPitch.value(), gyroRoll.value(), deltaXLeft, deltaYLeft, deltaZLeft);
			printf("\ngyroPitch: %f | gyroRoll: %f \n", gyroPitch.value(), gyroRoll.value());
			printf("xpCount: %d\n", xpCount);
			printf("reward: %f\n", reward);
			printf("State ");
			learnData[Side::LEFT].learnState.print();
			printf("Action ");
			learnData[Side::LEFT].action.print();
			printf("\n");
		}

	}

	// Save what was learned
	if ((xpCount % 100 == 0) && (xpCount > 0) && saveExperience && learn && false == neuralLearning.isSaving.load()) {
		neuralLearning.saveWhatWasLearnedAsync(parameterFileName);
		xpCount++;
	}

	rewardCount++;
	rewardSum = rewardSum + (1.0 / double(rewardCount)) * (getReward() - rewardSum);

}


/*------------------------------------------------------------------------------------------------*/


float LearningWalker::getReward() {
	// Reward based on gyroscope
	Degree gyroRoll = getGyroData().getRoll();
	Degree gyroPitch = getGyroData().getPitch() - leanOffsetGyroPitch;

	const float maxDegree = 2.5; // if gyro degree smaller than this, its considered as good
	const float maxReward = maxDegree * maxDegree * 2;
	const float minReward = -1000;

	// Reward based on motor loads
	//float sumOfLoads = 0;
	//sumOfLoads += abs(getMotorBus().getCurrentLoad(MOTOR_LEFT_HIP_ROLL)) % 1024;
	//sumOfLoads += abs(getMotorBus().getCurrentLoad(MOTOR_RIGHT_HIP_ROLL)) % 1024;
	//sumOfLoads += abs(getMotorBus().getCurrentLoad(MOTOR_LEFT_KNEE_TOP)) % 1024;
	//sumOfLoads += abs(getMotorBus().getCurrentLoad(MOTOR_RIGHT_KNEE_TOP)) % 1024;
	//sumOfLoads += abs(getMotorBus().getCurrentLoad(MOTOR_LEFT_KNEE_BOTTOM)) % 1024;
	//sumOfLoads += abs(getMotorBus().getCurrentLoad(MOTOR_RIGHT_KNEE_BOTTOM)) % 1024;
	//sumOfLoads = sumOfLoads / 50.0;
	//printf("time: %d, load: %f\n", currentTimeLeft, sumOfLoads);

	float error = gyroRoll.value() * gyroRoll.value() + gyroPitch.value() * gyroPitch.value();

	float reward = (maxReward - error);
	if (reward < minReward) reward = minReward;
	return reward;
}

