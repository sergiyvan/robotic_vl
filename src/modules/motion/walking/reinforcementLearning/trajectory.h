/*
 * Trajectory.h
 *
 *  Created on: Feb 7, 2014
 *      Author: stepuh
 */

#ifndef TRAJECTORY_H_
#define TRAJECTORY_H_


#include "modules/motion/walking/motion_walker_base.h"
#include "modules/motion/kinematic/kinematic2013.h"
#include "modules/motion/walking/reinforcementLearning/rlModel.h"


struct WalkingSpeeds {
	WalkingSpeeds()
		: x(0)
		, y(0)
		, yaw(0)
	{}

	double x;
	double y;
	double yaw;
};




class Trajectory {
public:

	double startingFactor;
	WalkingSpeeds currentSpeeds;
	WalkingSpeeds maxSpeeds;


	Trajectory();
	virtual ~Trajectory();

	EndEffectorPose getPose(Foot foot, int time, RL_Action oldState, RL_Action currentAction);


	double computePendulumY(const Foot foot, int time);

	double computePendulumZ(const Foot foot, int time);

	double computeXLift(Foot foot, int time, double deltaStart, double deltaEnd);

	double computeXSupport(Foot foot, int time, double deltaStart, double deltaEnd);

	double computeYLift(Foot foot, int time, double deltaStart, double deltaEnd);

	double computeYSupport(Foot foot, int time, double deltaStart, double deltaEnd);

	double computeZLift(Foot foot, int time);

	double computeZSupport(Foot foot, int time);

	double computeYawLift(Foot foot, int time, double start, double end);

	double computeYawSupport(Foot foot, int time, double start, double end);

	double computeRollLift(Foot foot, int time, double start, double end);

	double computeRollSupport(Foot foot, int time, double start, double end);

	void setSpeeds(double speedX, double speedY, double speedYaw);

	WalkingSpeeds getCurrentSpeeds();

	// Parameters
	double legLength;
	double stepHeight;
	double pendulumAmplitude;
	double maxForwardSpeed;
	double maxBackwardSpeed;
	double maxSidewardSpeed;
	double maxRotationSpeed;
	double innerYOffset;
	double yawOffset;
	double rollOffset;
	double pendulumFactorY;
	double suppressionZ;
	double xOffset;
	bool startKick;
	double xFactor;

private:

	int timeToStabilizeOnOtherFoot;
	int timeToLiftFoot;
	int timeToLowerFoot;
	int timeToStabilizeOnOwnFoot;


	double smoothMovementStartEnd(double distance, double duration, double currentTime, double offset);
	double smoothMovementStart(double distance, double duration, double currentTime, double offset);
	double smoothMovementEnd(double distance, double duration, double currentTime, double offset);
	double linearMovement(double distance, double duration, double currentTime, double offset);

};

#endif /* TRAJECTORY_H_ */
