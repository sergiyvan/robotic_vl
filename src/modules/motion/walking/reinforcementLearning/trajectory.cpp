/*
 * tajectory.cpp
 *
 *  Created on: Oct 27, 2013
 *      Author: stepuh
 */

#include "trajectory.h"
#include "utils/math/Math.h"


Trajectory::Trajectory()
	: startingFactor(0.0)
	, legLength(315)
	, stepHeight(35)
	, pendulumAmplitude(5.0)
	, maxForwardSpeed(0)
	, maxBackwardSpeed(0)
	, maxSidewardSpeed(0)
	, maxRotationSpeed(0)
	, innerYOffset(0.0)
	, yawOffset(0)
	, rollOffset(0)
	, pendulumFactorY(0)
	, suppressionZ(0.0)
	, xOffset(0)
	, startKick(false)
	, xFactor (0.3)

	, timeToStabilizeOnOtherFoot(45)
	, timeToLiftFoot(45)
	, timeToLowerFoot(45)
	, timeToStabilizeOnOwnFoot(45)

{
	// EMPTY!
}


/*------------------------------------------------------------------------------------------------*/


Trajectory::~Trajectory() {
	// EMPTY!
}


/*------------------------------------------------------------------------------------------------*/


EndEffectorPose Trajectory::getPose(Foot foot, int time, RL_Action oldAction, RL_Action currAction) {
	EndEffectorPose pose;
	time = time % 360;
	if (foot == LEFT_FOOT) {
		time = (time + 180) % 360;
	}

	// LIFT FOOT
	if (time < 180) {
		double startYaw = currentSpeeds.yaw;
		double endYaw   = currentSpeeds.yaw;

		pose.x    = computeXLift(foot, time, oldAction.getParameter("x") * 4, currAction.getParameter("x") * 4);
		pose.y    = computeYLift(foot, time, oldAction.getParameter("y") * 4, currAction.getParameter("y") * 4);
		pose.z    = computeZLift(foot, time);
		pose.yaw  = computeYawLift(foot, time, startYaw, endYaw);
		pose.roll = computeRollLift(foot, time, 0, 0);



	// SUPPORT FOOT
	} else {

		double startYaw = 0;
		double endYaw   = 0;

		pose.x   = computeXSupport(foot, time, oldAction.getParameter("x") * 4, currAction.getParameter("x") * 4);
		pose.y   = computeYSupport(foot, time, oldAction.getParameter("y") * 4, currAction.getParameter("y") * 4);
		pose.z   = computeZSupport(foot, time);
		pose.yaw = computeYawSupport(foot, time, startYaw, endYaw);
		pose.roll = computeRollSupport(foot, time, 0, 0);
	}

	return pose;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::smoothMovementStartEnd(double distance, double duration, double currentTime, double offset) {
	double fraction = 1.0;
	if (duration != 0.0) {
		fraction = 180.0 / duration;
	}
	return (0.5 * (-cos(currentTime * fraction * Math::pi_180) + 1)) * distance + offset;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::smoothMovementStart(double distance, double duration, double currentTime, double offset) {
	double fraction = 1.0;
	if (duration != 0.0) {
		fraction = 90.0 / duration;
	}
	return (-cos(currentTime * fraction * Math::pi_180) + 1) * distance + offset;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::smoothMovementEnd(double distance, double duration, double currentTime, double offset) {
	double fraction = 1.0;
	if (duration != 0.0) {
		fraction = 90.0 / duration;
	}
	return sin(currentTime * fraction * Math::pi_180) * distance + offset;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::linearMovement(double distance, double duration, double currentTime, double offset) {
	double fraction = 1.0;
	if (duration != 0.0) {
		fraction = currentTime / duration;
	}
	return fraction * distance + offset;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computePendulumY(const Foot foot, int time) {
	double pendulumAngle = pendulumAmplitude * sin(time * Math::pi_180);
	return legLength * cos((90.0 - pendulumAngle) * Math::pi_180) * pendulumFactorY;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computePendulumZ(const Foot foot, int time) {
	//if (foot == LEFT_FOOT) {
	//	time = (time + 180) % 360;
	//}

	// The pendulum amplitude differs depending on whether the current foot is being lifted or is supporting
	double amplitude = pendulumAmplitude;
	const double factorLift = 1.0;//4
	const double factorSupport = 0.0; // 1.0
	if (time >= 180) {
		amplitude = pendulumAmplitude * factorSupport;
	} else {
		amplitude = pendulumAmplitude * factorLift;
	}

	// We compute the pendulum angle for the given amplitude.
	// By manipulating <factor>, the z-movement relative to the y-movement changes.
	double pendulumAngle = amplitude * sin(time * Math::pi_180);

	return (legLength * sin((90.0 - pendulumAngle) * Math::pi_180));
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeXLift(Foot foot, int time, double deltaStart, double deltaEnd) {
	double value = 0; // What is being returned

	double start = (-currentSpeeds.x * 2  + deltaStart) * startingFactor;
	double end   = (currentSpeeds.x + deltaEnd) * startingFactor;

	double deltaX = end - start;
	//double distanceStabilize = 0.25 * deltaX; // magic
	//double distanceStabilize = timeToStabilizeOnOtherFoot * deltaX / (timeToLiftFoot + timeToLowerFoot) + deltaX / 2;
	double distanceStabilize = -deltaX * timeToStabilizeOnOtherFoot / 360;
	distanceStabilize = 0;

	double distanceLift      = deltaX + 2 * distanceStabilize;

	// Values needed in each period for calculating movement
	double distance = 0;
	double duration = 0;
	double currTime = 0;
	double offset   = 0;

	double xOffset = this->xOffset;
	if (deltaX > 0) {
		//xOffset += deltaX * 0.75;
		xOffset += deltaX * xFactor;//0.25;
	} else {
		xOffset += deltaX * 0.1;
	}

	//if (!startKick) {
		value += xOffset;
	//}

	timeToStabilizeOnOtherFoot = 00;//00
	timeToLiftFoot = 50;//50
	timeToLowerFoot = 50;//50
	timeToStabilizeOnOwnFoot = 80;//80

	// TOO EARLY
	if (time <= 0) {
		value += start;

	// STABALIZE ON OTHER FOOT
	} else if ((time > 0) && (time <= timeToStabilizeOnOtherFoot)) { // FIXME: with timeToStabilizeOnOtherFoot == 0 this is never entered
		// precondition: value = start
		distance = -distanceStabilize;
		duration = timeToStabilizeOnOtherFoot;
		currTime = time;
		offset   = start;
		value += smoothMovementEnd(distance, duration, currTime, offset);
		// postcondition: value = (start - distanceStabilize)

	// LIFT AND LOWER THIS FOOT
	} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
		// precondition: value = (start - distanceStabilize)
		distance = distanceLift;
		duration = timeToLiftFoot + timeToLowerFoot;
		currTime = time - timeToStabilizeOnOtherFoot;
		offset   = start - distanceStabilize;
		value += smoothMovementStartEnd(distance, duration, currTime, offset);
		//value += smoothMovementEnd(distance, duration, currTime, offset);

		// postcondition: value = start -distanceStabilize + distanceLift

	// STABILIZE ON OWN FOOT
	} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot) && (time <= 180)) {
		// precondition: value = start -distanceStabilize + distanceLift;
		distance = -distanceStabilize;
		duration = timeToStabilizeOnOwnFoot;
		currTime = time - (timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot);
		offset   = start - distanceStabilize + distanceLift;
		value += smoothMovementStart(distance, duration, currTime, offset);
		// postcondition: value = start + distanceThisFoot - 2 * distanceStabilize;

	// TOO LATE
	} else {
		value += end;
	}


	return value;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeXSupport(Foot foot, int time, double deltaStart, double deltaEnd) {
	double value = 0;          // What is being returned.

	double start = (currentSpeeds.x + deltaStart) * startingFactor;
	double end   = (-currentSpeeds.x * 2 + deltaEnd) * startingFactor;

	double deltaX = abs(end - start);
	deltaX = start-end;
	//double distanceStabilize = 0.25 * deltaX; // magic
	//double distanceStabilize = timeToStabilizeOnOtherFoot * deltaX / (timeToLiftFoot + timeToLowerFoot) + deltaX / 2;
	double distanceStabilize = -deltaX * timeToStabilizeOnOtherFoot / 360;
	distanceStabilize = 0;

	double distanceSupport   = deltaX - 2 * distanceStabilize;

	// Values needed in each period for calculating movement
	double distance = 0;
	double duration = 0;
	double currTime = 0;
	double offset   = 0;


	double xOffset = this->xOffset;
	if (deltaX > 0) {
		//xOffset += deltaX * 0.75;
		xOffset += deltaX * xFactor;//0.25;
	} else {
		xOffset += deltaX * 0.1;
	}

	//if (!startKick) {
		value += xOffset;
	//}

	timeToStabilizeOnOtherFoot = 00;//00 all
	timeToLiftFoot = 60;//60
	timeToLowerFoot = 60;//60
	timeToStabilizeOnOwnFoot = 60;//60



	time -= 180;

	// TOO EARLY
	if (time <= 0) {
		value = +start;

	// STABALIZE ON THIS FOOT
	} else if ((time > 0) && (time <= timeToStabilizeOnOtherFoot)) { // FIXME: with timeToStabilizeOnOtherFoot == 0 this is never entered
		// precondition: value = start
		distance = -distanceStabilize;
		duration = timeToStabilizeOnOtherFoot;
		currTime = time;
		offset   = start;
		value += linearMovement(distance, duration, currTime, offset);
		// postcondition: value = start - distanceStabilize

	// LIFT AND LOWER OTHER FOOT
	} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
		// precondition: value = start - distanceStabilize
		distance = -distanceSupport;
		duration = timeToLiftFoot + timeToLowerFoot;
		currTime = time - timeToStabilizeOnOtherFoot;
		offset   = start - distanceStabilize;
		value += linearMovement(distance, duration, currTime, offset);
		// postcondition: value = start - distanceStabilize - distanceSupport;

	// STABILIZE ON OTHER FOOT
	} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot) && (time <= 180)) {
		// precondition: value = start - distanceStabilize - distanceSupport;
		distance = -distanceStabilize;
		duration = timeToStabilizeOnOwnFoot;
		currTime = time - (timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot);
		offset   = start - distanceStabilize - distanceSupport;
		value += linearMovement(distance, duration, currTime, offset);
   		// postcondition: value = start - distanceSupport - 2 * distanceStabilize;

	// TOO LATE
	} else {
		value = +end;
	}

	return value;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeYLift(Foot foot, int time, double deltaStart, double deltaEnd) {
	double deltaY = currentSpeeds.y;
	deltaY *= 0.5;


	// Basic movement is pendulum based to keep center of mass always above supporting foot
	double value = computePendulumY(foot, time);
	value += innerYOffset;
	//value += 10;

	double distanceLift      = std::abs(deltaY) * 2.0;//2.0
	double distanceSupport   = std::abs(deltaY) * 1.5;//1.0
	double distanceStabilize = std::abs(deltaY) * 0.0;//0.0
	double distanceInner     = std::abs(deltaY) * 0.0;//0.0
	double distanceOuter     = std::abs(deltaY) * 1.0;//1.0

	double distance = 0;
	double duration = 0;
	double currTime = 0;
	double offset   = 0;

	timeToStabilizeOnOtherFoot = 40;
	timeToLiftFoot = 30;
	timeToLowerFoot = 30;
	timeToStabilizeOnOwnFoot = 80;

	timeToStabilizeOnOtherFoot = 30;//45 all
	timeToLiftFoot = 30;
	timeToLowerFoot = 30;
	timeToStabilizeOnOwnFoot = 90;

	timeToStabilizeOnOtherFoot = 30;
	timeToLiftFoot = 35;
	timeToLowerFoot = 35;
	timeToStabilizeOnOwnFoot = 80;

	timeToStabilizeOnOtherFoot = 30;
	timeToLiftFoot = 30;//30
	timeToLowerFoot = 60;//60
	timeToStabilizeOnOwnFoot = 60;


	//--

//	timeToStabilizeOnOtherFoot = 35;
//	timeToLiftFoot = 55;
//	timeToLowerFoot = 55;
//	timeToStabilizeOnOwnFoot = 35;

	// THE CURRENT FOOT IS IN WALKING DIRECTION
	if (((foot == RIGHT_FOOT) && (deltaY <= 0)) || ((foot == LEFT_FOOT) && (deltaY >= 0))) {

		// TOO EARLY
		if (time <= 0) {
			value += deltaStart + distanceOuter;

		// STABALIZE ON OTHER FOOT
		}else if ((time > 0) && (time <= timeToStabilizeOnOtherFoot)) {
			distance = distanceStabilize;
			duration = timeToStabilizeOnOtherFoot;
			currTime = time;
			offset   = deltaStart + distanceOuter;
			value += linearMovement(distance, duration, currTime, offset);
			// -> value: pendulumY

		// LIFT AND LOWER CURRENT FOOT
//		} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
//			distance = distanceLift - deltaStart + deltaEnd;
//			duration = timeToLiftFoot + timeToLowerFoot;
//			currTime = time - timeToStabilizeOnOtherFoot;
//			offset   = distanceStabilize + deltaStart;
//			value += smoothMovementEnd(distance, duration, currTime, offset);
//			// -> value: pendulumY + distanceLift

		// LIFT CURRENT FOOT
		} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot)) {
			distance = distanceLift - deltaStart + deltaEnd;
			duration = timeToLiftFoot;
			currTime = time - timeToStabilizeOnOtherFoot;
			offset   = distanceStabilize + deltaStart + distanceOuter;
			value += smoothMovementEnd(distance, duration, currTime, offset);

		// LOWER CURRENT FOOT
		} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
			value += distanceStabilize + deltaEnd + distanceLift + distanceOuter;


		// STABALIZE ON CURRENT FOOT
		}else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLiftFoot) && (time <= 180)) {
			distance = -(distanceLift + distanceStabilize);
			duration = timeToStabilizeOnOwnFoot;
			currTime = time - (timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot);
			offset   = distanceLift + distanceStabilize + deltaEnd + distanceOuter;
			value += smoothMovementStart(distance, duration, currTime, offset);
			// -> value -> pendulumY

		// TOO LATE
		} else {
			value += deltaEnd + distanceOuter;
		}


	// THE CURRENT FOOT IS NOT IN WALKING DIRECTION
	} else {

		// TOO EARLY
		if (time <= 0) {
			value += deltaStart + distanceSupport + distanceLift;

			// STABALIZE ON OTHER FOOT
		} else if ((time > 0) && (time <= timeToStabilizeOnOtherFoot)) {
			distance = distanceStabilize;
			duration = timeToStabilizeOnOtherFoot;
			currTime = time;
			offset   = deltaStart + distanceSupport + distanceLift;
			value += smoothMovementEnd(distance, duration, currTime, offset);

		/*
		// LIFT AND LOWER CURRENT FOOT
		} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
			distance = -(distanceLift + distanceSupport + deltaStart + distanceInner) + deltaEnd;
			duration = timeToLiftFoot + timeToLowerFoot;
			currTime = time - timeToStabilizeOnOtherFoot;
			offset   = distanceSupport + distanceLift + deltaStart  +distanceStabilize;
			value += smoothMovementStart(distance, duration, currTime, offset);
		*/

		// LIFT CURRENT FOOT
		} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot)) {
			distance = -(distanceLift + distanceSupport + deltaStart + distanceInner) + deltaEnd;
			duration = timeToLiftFoot;
			currTime = time - timeToStabilizeOnOtherFoot;
			offset   = distanceSupport + distanceLift + deltaStart +distanceStabilize;
			value += smoothMovementEnd(distance, duration, currTime, offset);

		// LOWER CURRENT FOOT
		} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
			value += distanceStabilize + deltaEnd -distanceInner;


		// STABALIZE ON CURRENT FOOT
		}else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLiftFoot) && (time <= 180)) {
			distance = -distanceStabilize;
			duration = timeToStabilizeOnOwnFoot;
			currTime = time - (timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot);
			offset   = distanceStabilize + deltaEnd -distanceInner;
			value += linearMovement(distance, duration, currTime, offset);

		// TOO LATE
		} else {
			value += deltaEnd - distanceInner;
		}
	}


	// The trajectories are symmetric to the z-axis
	if (foot == LEFT_FOOT) {
		value *= -1.0;
	}


	value *= startingFactor;
	return value;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeYSupport(Foot foot, int time, double deltaStart, double deltaEnd) {
	double deltaY = currentSpeeds.y;
	deltaY *= 0.5;


	// Basic movement is pendulum based to keep center of mass always above supporting foot
	double value = computePendulumY(foot, time);
	value += innerYOffset;
	//value -= 10;


	double distanceLift      = std::abs(deltaY) * 2.0;//2
	double distanceSupport   = std::abs(deltaY) * 1.5;//1.0
	double distanceStabilize = std::abs(deltaY) * 0.0;//0.0
	double distanceInner     = std::abs(deltaY) * 0.0;//0.0
	double distanceOuter     = std::abs(deltaY) * 1.0;//1.0


	double distance = 0;
	double duration = 0;
	double currTime = 0;
	double offset   = 0;

	timeToStabilizeOnOtherFoot = 40;//60
	timeToLiftFoot = 30;
	timeToLowerFoot = 30;
	timeToStabilizeOnOwnFoot = 80;

	timeToStabilizeOnOtherFoot = 30;
	timeToLiftFoot = 30;
	timeToLowerFoot = 30;
	timeToStabilizeOnOwnFoot = 90;

	timeToStabilizeOnOtherFoot = 30;
	timeToLiftFoot = 35;
	timeToLowerFoot = 35;
	timeToStabilizeOnOwnFoot = 80;

	timeToStabilizeOnOtherFoot = 30;
	timeToLiftFoot = 45;
	timeToLowerFoot = 45;
	timeToStabilizeOnOwnFoot = 60;

	//--

	//timeToStabilizeOnOtherFoot = 35;
	//timeToLiftFoot = 55;
	//timeToLowerFoot = 55;
	//timeToStabilizeOnOwnFoot = 35;


	time -= 180;

	// THE CURRENT FOOT IS IN WALKING DIRECTION
	if (((foot == RIGHT_FOOT) && (deltaY <= 0)) || ((foot == LEFT_FOOT) && (deltaY >= 0))) {

		// TOO EARLY
		if (time <= 0) {
			value += deltaStart + distanceOuter;

		// STABALIZE ON THIS FOOT
		} else if ((time > 0) && (time <= timeToStabilizeOnOwnFoot)) {
			distance = -distanceStabilize;
			duration = timeToStabilizeOnOwnFoot;
			currTime = time;
			offset   = deltaStart + distanceOuter;
			value += smoothMovementEnd(distance, duration, currTime, offset);

		// LIFT AND LOWER OTHER FOOT
		} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
			distance = -deltaStart + deltaEnd;
			duration = timeToLiftFoot + timeToLowerFoot;
			currTime = time - timeToStabilizeOnOtherFoot;
			offset   = deltaStart -distanceStabilize + distanceOuter;
			value += smoothMovementStartEnd(distance, duration, currTime, offset);

		// STABALIZE ON BOTH FEET
		} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot) && (time <= 180)) {
			distance = distanceStabilize;
			duration = timeToStabilizeOnOtherFoot;
			currTime = time - (timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot);
			offset   = -distanceStabilize + deltaEnd + distanceOuter;
			value += smoothMovementStart(distance, duration, currTime, offset);

		// TOO LATE
		} else {
			value += deltaEnd + distanceOuter;
		}


	// THE CURRENT FOOT IS NOT IN WALKING DIRECTION
	} else {

		// TOO EARLY
		if (time <= 0) {
			value += deltaStart -distanceInner;

		// STABALIZE ON THIS FOOT
		} else if ((time > 0) && (time <= timeToStabilizeOnOwnFoot)) {
			distance = -distanceStabilize;
			duration = timeToStabilizeOnOwnFoot;
			currTime = time;
			offset   = deltaStart -distanceInner;
			value += smoothMovementEnd(distance, duration, currTime, offset);

///*
		// LIFT AND LOWER OTHER FOOT
		} else if ((time > timeToStabilizeOnOwnFoot) && (time <= timeToStabilizeOnOwnFoot + timeToLiftFoot + timeToLowerFoot)) {
			distance = distanceSupport - deltaStart + deltaEnd + distanceInner;
			duration = timeToLiftFoot + timeToLowerFoot;
			currTime = time - timeToStabilizeOnOwnFoot;
			offset   = -distanceStabilize + deltaStart - distanceInner;
			value += smoothMovementStart(distance, duration, currTime, offset);
//*/

/*
		// LIFT OTHER FOOT
		} else if ((time > timeToStabilizeOnOwnFoot) && (time <= timeToStabilizeOnOwnFoot + timeToLiftFoot)) {
			distance = distanceSupport - deltaStart + deltaEnd + distanceInner;
			duration = timeToLiftFoot;
			currTime = time - timeToStabilizeOnOwnFoot;
			offset   = -distanceStabilize + deltaStart - distanceInner;
			value += smoothMovementStart(distance, duration, currTime, offset);

		// LOWER OTHER FOOT
		} else if ((time > timeToStabilizeOnOwnFoot + timeToLiftFoot) && (time <= timeToStabilizeOnOwnFoot + timeToLiftFoot + timeToLowerFoot)) {
			 value += distanceSupport - distanceStabilize + deltaEnd;
*/

		// STABALIZE ON OTHER FOOT
		} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot) && (time <= 180)) {
			distance = distanceLift + distanceStabilize;
			duration = timeToStabilizeOnOtherFoot;
			currTime = time - (timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot);
			offset   = distanceSupport - distanceStabilize + deltaEnd;
			value += linearMovement(distance, duration, currTime, offset);

		// TOO LATE
		} else {
			value += deltaEnd + distanceSupport + distanceLift;
		}
	}


	// The trajectories are symmetric to the z-axis
	if (foot == LEFT_FOOT) {
		value *= -1.0;
	}

	value *= startingFactor;
	return value;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeZLift(Foot foot, int time) {
	double value    = 0;                // what is being returned.

	timeToStabilizeOnOtherFoot = 35;//30 all
	timeToLiftFoot = 60;//70;//75;//80
	timeToLowerFoot = 60;;//35;//40
	timeToStabilizeOnOwnFoot = 35;//30

	value = legLength - computePendulumZ(foot, time);


	if (maxSpeeds.x != 0) {
		double suppression = suppressionZ * currentSpeeds.x / maxSpeeds.x;
		value += linearMovement(suppression, 180, time, 0);
	}


	double height = stepHeight + 1.5 * std::abs(currentSpeeds.x); //2
	if (((foot == RIGHT_FOOT) && (currentSpeeds.y <= 0)) || ((foot == LEFT_FOOT) && (currentSpeeds.y >= 0))) {
		height += std::abs(currentSpeeds.y) * 1;
	}
	//height = Math::limited(height, 0.0, stepHeight + std::abs(currentSpeeds.x));


	if (time <= 0) {
		value += 0;

	// STABALIZE ON OTHER FOOT
	} else if ((time > 0) && (time <= timeToStabilizeOnOtherFoot)) {
		value += 0;

	// LIFT CURRENT FOOT
	} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot)) {
		double fraction = 90.0 / timeToLiftFoot;
		value += sin((time - timeToStabilizeOnOtherFoot) * fraction * Math::pi_180) * height;

	// LOWER CURRENT FOOT
	} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
		double distance = -height;
		double duration = timeToLowerFoot;
		double currTime = time - (timeToStabilizeOnOtherFoot + timeToLiftFoot);
		double offset   = height;
		value += smoothMovementStartEnd(distance, duration, currTime, offset);

	// STABILIZE ON OWN FOOT
	} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot) && (time <= 180)) {
		value += 0;

	// TOO LATE
	} else {
		value += 0;
	}

	if (value > 60) {
		value = 60;
	}

	value *= startingFactor;

	value = Math::limited(value, 0.0, 80.0);
	return (legLength - value);
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeZSupport(Foot foot, int time) {
	double value = legLength - computePendulumZ(foot, time);
	value *= startingFactor;

//	if (maxSpeeds.x != 0) {
//		double suppression = suppressionZ * std::abs(currentSpeeds.x) / maxSpeeds.x;
//		value += linearMovement(-suppression, 180, time, suppression);
//	}
	return legLength - value;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeYawLift(Foot foot, int time, double deltaStart, double deltaEnd) {
	double value = 0; // What is being returned

	double offset = +yawOffset;
	if (foot == LEFT_FOOT) {
		offset = -offset;
	}

	value += offset;

	double speed = -currentSpeeds.yaw * 0.25;

	timeToStabilizeOnOtherFoot = 35;
	timeToLiftFoot = 55;
	timeToLowerFoot = 55;
	timeToStabilizeOnOwnFoot = 35;


	// THE CURRENT FOOT IS IN WALKING DIRECTION
	//if (((foot == RIGHT_FOOT) && (currentSpeeds.yaw <= 0)) || ((foot == LEFT_FOOT) && (currentSpeeds.yaw >= 0))) {

		// TOO EARLY
		if (time <= 0) {
			value += 0;

		// STABALIZE ON OTHER FOOT
		} else if ((time > 0) && (time <= timeToStabilizeOnOtherFoot)) {
			value += 0;

		// LIFT AND LOWER THIS FOOT
		} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
			double distance = speed;
			double duration = timeToLiftFoot + timeToLowerFoot;
			double currTime = time - timeToStabilizeOnOtherFoot;
			double offset   = 0;
			value += smoothMovementStartEnd(distance, duration, currTime, offset);

		// STABILIZE ON OWN FOOT
		} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot) && (time <= 180)) {
			value += speed;

		// TOO LATE
		} else {
			value += speed;
		}
	//}

	return value * startingFactor;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeYawSupport(Foot foot, int time, double deltaStart, double deltaEnd) {
	double value = 0;
	double speed = -currentSpeeds.yaw * 0.25;

	double offset = +yawOffset;
	if (foot == LEFT_FOOT) {
		offset = -offset;
	}
	value += offset;

	time -= 180;

	timeToStabilizeOnOtherFoot = 35; //50,40
	timeToLiftFoot = 55;
	timeToLowerFoot = 55;
	timeToStabilizeOnOwnFoot = 35;


	// THE CURRENT FOOT IS IN WALKING DIRECTION
	//if (((foot == RIGHT_FOOT) && (currentSpeeds.yaw <= 0)) || ((foot == LEFT_FOOT) && (currentSpeeds.yaw >= 0))) {

		// TOO EARLY
		if (time <= 0) {
			value += speed;

		// STABALIZE ON OTHER FOOT
		} else if ((time > 0) && (time <= timeToStabilizeOnOtherFoot)) {
			value += speed;

		// LIFT AND LOWER THIS FOOT
		} else if ((time > timeToStabilizeOnOtherFoot) && (time <= timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot)) {
			double distance = -speed;
			double duration = timeToLiftFoot + timeToLowerFoot;
			double currTime = time - timeToStabilizeOnOtherFoot;
			double offset   = currentSpeeds.yaw;
			value += smoothMovementStartEnd(distance, duration, currTime, offset);

		// STABILIZE ON OWN FOOT
		} else if ((time > timeToStabilizeOnOtherFoot + timeToLiftFoot + timeToLowerFoot) && (time <= 180)) {
			value += 0;

		// TOO LATE
		} else {
			value += 0;
		}
	//}

	return value * startingFactor;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeRollLift(Foot foot, int time, double deltaStart, double deltaEnd) {
	double value = 0;

	value += rollOffset;

	if (foot == LEFT_FOOT) {
		value *= -1;
	}

	value += 5 * currentSpeeds.y;

	return value;
}


/*------------------------------------------------------------------------------------------------*/


double Trajectory::computeRollSupport(Foot foot, int time, double deltaStart, double deltaEnd) {
	double value = 0;

	value += rollOffset;

	if (foot == LEFT_FOOT) {
		value *= -1;
	}


	return value;
}


/*------------------------------------------------------------------------------------------------*/


void Trajectory::setSpeeds(double speedX, double speedY, double speedYaw) {
	currentSpeeds.x   = Math::limited(speedX,   -maxBackwardSpeed, maxForwardSpeed);
	currentSpeeds.y   = Math::limited(speedY,   -maxSidewardSpeed, maxSidewardSpeed);
	currentSpeeds.yaw = Math::limited(speedYaw, -maxRotationSpeed, maxRotationSpeed);
}


/*------------------------------------------------------------------------------------------------*/


WalkingSpeeds Trajectory::getCurrentSpeeds() {
	return currentSpeeds;
}


