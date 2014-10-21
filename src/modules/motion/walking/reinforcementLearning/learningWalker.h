#ifndef LEARNINGWALKER_H_
#define LEARNINGWALKER_H_

#include "modules/motion/walking/motion_walker_base.h"
#include "utils/math/Pose2D.h"


#include "platform/system/events.h"

#include "modules/motion/kinematic/kinematic2013.h"

#include "modules/motion/walking/mw_smoother.h"
#include "modules/motion/walking/mw_starter.h"
#include "modules/motion/walking/mw_timer.h"

#include "representations/motion/motionRequest.h"
#include "representations/motion/motionStatus.h"
#include "representations/motion/activeMotion.h"
#include "representations/motion/supportFoot.h"
#include "representations/motion/motorPositionRequest.h"
#include "representations/motion/walkerTargetValues.h"

#include "representations/hardware/gyroData.h"
//#include "representations/posture.h"

#include "ModuleFramework/Module.h"

#include "trajectory.h"

#include "rlModelFactory.h"
#include "rlREPATD.h"

#include "modules/motion/walking/supervisedLearning/slNeuronalNetwork.h"
//#include "neuralLearning.h"
#include "modules/motion/walking/reinforcementLearning/neuralLearning.h"

/**
 * @defgroup walker11 The Walker
 * @ingroup motions
 *
 * The Walker is the system responsible for the locomotion of the robots (model
 * 2011). It is based on a central pattern generator. For further theoretical
 * information have a look at the master's thesis of Johannes (to be published
 * in 2011).
 * <br/>
 * Some technical information:
 * <br/>
 * The central timing is done with two MW_Timer timers, which move on from 0 to 360,
 * since some trigonometric functions are used. How long one degree is, is defined
 * by the stepsPerSecond variable.
 * <br/>
 * The movement of the legs is determined by the x, y and yaw speed. X positive
 * is forward, y positive is to the left and yaw positive is counter-clockwise.
 * There are two different methods, either sine or linear movement. All those
 * pattern generation is done in the compute?Pos() functions.
 * <br/>
 * The timing is central. Most of it is done in MotionWalker2011::stepSync().
 * A new step is started, when the other step is finished. This is recognized by
 * foot pressure sensors (load cells). On an impact the timer of the other foot
 * is reset, and therefore starts the new step. Thus the double support phase
 * normally is minimized. Under some circumstances we wait for the step (e.g.
 * too early impacts).
 * <br/>
 * The static interface from the motion_walker_base.cpp comes from the old microcontroller
 * code and should be changed in the future.
 * <br/>
 * The speeds set by this interface are scaled and filtered in the computeSpeeds()
 * function.
 * <br/>
 * The starting of the walker is done by a MW_Starter, which moves from 0.0 to 1.0.
 * This factor is multiplied to the step height. So in the beginning the staeps are
 * less heigh.
 * <br/>
 * Stopping the walker is done by finishing the next step. This means that up to
 * 2 steps are performed, where the last step is done with speeds = 0.
 * @{
 */

class Kinematic;
struct EndEffectorPose;


BEGIN_DECLARE_MODULE(LearningWalker)
	REQUIRE(ActiveMotion)
//	REQUIRE(RobotPosture)
	REQUIRE(GyroData)
	REQUIRE(MotionRequest)

	PROVIDE(WalkerTargetValues)
	PROVIDE(MotionStatus)
	PROVIDE(MotorPositionRequest)
	PROVIDE(SupportFoot)
END_DECLARE_MODULE(LearningWalker)


/**
 * The actual Walker class. See @ref walker for technical details of the walking
 * system.
 */

class LearningWalker
	: public LearningWalkerBase
	, public MotionWalker
	, EventCallback
{
public:
	LearningWalker();
	virtual ~LearningWalker();

	virtual void init() override;
	virtual void execute() override;
	virtual void eventCallback(EventType eventType, void* data) override;

	enum class Side { LEFT, RIGHT };

private:

	void initLearning();


	void handleInstability();


	/**
	 * Computes the speed of the given foot. The speed is first scaled and then
	 * filtered by a running average filter. Afterwards it is cut of by a convex
	 * hull computed by checkMaxSpeed().
	 * @param foot The foot for which the speed should be computed.
	 */
	void computeSpeeds(Side side);


	/**
	 * Computes the time based pattern of the spine roll.
	 * @return the roll rotation at current time for the spine
	 *
	 * !TODO this function needs to be rewritten. timer shouldn't be a
	 * parameter, the problem is that MW_Timer is far away from anykind of
	 * constcorrectnis

	 */
	Degree computeSpineRoll(int currentTimeLeft, int currentTimeRight) const;
	Degree computeStomachPitch() const;


	/**
	 * Computes the gyroscope based stabilization pitch-rotation of the current arm.
	 * If we fall forward, we move the arms back, otherwise we move them forward.
	 * @param arm The arm to be moved
	 * @return additional pitch rotation for the current arm
	 */
	Degree computeStabilizationArmPitch(Side side) const;


	/**
	 * Computes the gyroscope based stabilization roll-rotation of the current arm.
	 * If we fall in the opposite direction of the arm we move the arm outside. Otherwise we move it inside.
	 * @param arm The arm to be moved
	 * @return additional roll rotation for the current arm
	 */
	Degree computeStabilizationArmRoll(Side side) const;

	Degree computeFootRoll(Side side) const;

	/**
	 * The arms should move during walking. They move symmetric to the x-axis
	 * movement of the legs.
	 * @param foot The corresponding foot to the arm
	 * @return The additional pitch-rotation of the arm
	 *
	 * !TODO this function needs to be rewritten. timer shouldn't be a
	 * parameter, the problem is that MW_Timer is far away from anykind of
	 * constcorrectnis
	 */
	Degree computeShoulder(Side side, int currentTime) const;

	/**
	 * All the specific movements of special motors are computed in this function.
	 * @param goal The goal position vector of all services.getMotors().
	 *
	 * !TODO this function needs to be rewritten. timer shouldn't be a
	 * parameter, the problem is that MW_Timer is far away from anykind of
	 * constcorrectnis
	 */
	std::map<MotorID, Degree> nonIKMovements(std::map<MotorID, Degree> retGoal, std::pair<int, int> timer) const;


	/**
	 * Synchronizes the steps. This is a crucial function. It recognizes impacts
	 * of the feet and starts steps accordingly.
	 * Since this function recognize the impacts instead of just ground contacts,
	 * it is also the place to trigger kicks and the stopping.
	 * It also computes a center of mass shifter, if impacts are late.
	 * @param foot The foot to synchronize
	 * @return true on an impact, false otherwise
	 */
	bool stepSync(Side side);


	/**
	 * Starts the starting factor. This actually does not start the walking itself!
	 */
	void startWalking();


	/**
	 * The current pattern time of either the left or right foot. This uses old
	 * saved data to get a consistent time over one frame.
	 * @param foot The foot which time to get
	 * @param return the timer of the other foot
	 * @return The time in degrees of the given foot
	 */
	inline int getTime(Side side) {
		return int(timer[side].timer.getTime());
	}

	void updateLearning();

	void computeFootPoses();

	void handleFallingDown();

	float getReward();


	//- PARAMETERS ------------------------------------------------------------

	double stepsPerSecond;
	int stepHeight;
	int orthoStepWidth;
	int legLength;
	int showStateInformation;
	int learn;
	int loadExperience;
	int saveExperience;
	double discountFactor;
	double learningRate;
	double explorationProbability;
	Degree pendulumAmplitude;
	int innerYOffset;
	double xOffset;
	double yawOffset;
	RPM motorSpeeds;
	double pendulumFactorY;
	double spineRollFactor;

	bool speedForMiniKickSet;
	double diffLegLength;

	std::string parameterFileName;

	//- MEMBERS ----------------------------------------------------------------

	Kinematic* ik;

	// EndEffectorPoses
	EndEffectorPose left_leg_pose;
	EndEffectorPose right_leg_pose;

	// idle values
	std::map<MotorID, Degree> idle;


	// Timer for each leg
	struct Timer {
		MW_Timer timer;
	};
	std::map<Side, Timer> timer;

	std::map<Side, MW_Timer> lastImpact;

	MW_Starter starter;

	struct Speed {
		int x, y, yaw;
	};
	std::map<Side, Speed> speed;

	bool startKick;

	bool useReinforcementLearning;    // true if (learn == true || loadExperience = true)
	double smoothedXSpeed;
	double smoothedYSpeed;

	double maxFootDrift;

	Degree footRoll;

	bool stopWalkingFlag;

	int  maxForwardSpeed;
	int  maxBackwardSpeed;
	int  maxSidewardSpeed;
	int  maxRotationSpeed;

	double stomachOffset;

	double timeToLiftFoot;
	double timeToStabilizeOnOtherFoot;

	Degree leanOffsetGyroPitch;

	struct LearnData {
		RL_State learnState;
		RL_State oldLearnState;
		RL_Action action;
		RL_Action oldAction;
	};
	std::map<Side, LearnData> learnData;

	// Learning Stuff

	RL_REPATD neuralLearning;

	Trajectory trajectory;

	double rewardSum;
	int rewardCount;

	int xpCount;
	int lastSampledTime;

	bool quitWalkerFlag;
	double slowDownFactor;
	double maxGyro;
	double stabilityYFactor;
	bool speedsWereSet;


	float instableAngle;
	robottime_t timeLastNotStable;
	robottime_t timeToStabilize;

	double suppressionZ;
	double stomachFactor;
};

#endif

