/*
 * Kinematic.h
 *
 *  Created on: 28.01.2011
 *      Author: johannes
 */

#ifndef KINEMATIC_H
#define KINEMATIC_H

#include "platform/hardware/robot/motorIDs.h"
#include "utils/units.h"

#include <map>

/*------------------------------------------------------------------------------------------------*/
/**
 * @defgroup kinematicModule The Kinematic Module
 * @ingroup motions
 *
 * The Kinematic Module is responsible for the translation of the pose of an
 * endeffector to the servo positions.
 *
 * @{
 */

/**
 * Represents a (simplified) pose of an endeffector. The origin is in the upper
 * most motor (e.g. in Robot2012 this is the center of the HIP_ROLL axis)
 *
 * The unit of x, y and z is mm;
 * the unit of yaw is degree.
 */

struct EndEffectorPose {
	int x;
	int y;
	int z;
	int yaw;
	int roll;
};

/*------------------------------------------------------------------------------------------------*/


/**
 * Simple names for the four end effectors in our robots
 */

enum EndEffector {
	LEFT_LEG  = 0,//!< LEFT_LEG
	RIGHT_LEG = 1,//!< RIGHT_LEG
};

typedef EndEffector SupportLeg;

/**
 * The Kinematic is the abstract base class for all forward and
 * inverse kinematic solutions.
 * Using this model we can run motions on different kind of robots.
 *
 * It is important that each solution uses the same base position for an end effector.
 */

class Kinematic {
public:
	Kinematic();
	virtual ~Kinematic() {};


	/**
	 * Calculates the inverse kinematic for a given list of end effectors and postures.
	 * It must be ensured, that postures and effectors have the same order. Note: If the same
	 * end effector appears several times, the behavior of this method is not defined.
	 *
	 * @return A map holding the end position of all involved services.getMotors().
	 */
    virtual std::map<MotorID, Degree> setEndEffectors(const EndEffectorPose& pos_left, const EndEffectorPose& pos_right) const;

protected:

	virtual std::map<MotorID, Degree> inverseLegKinematics(const EndEffectorPose& posture, const EndEffector effector) const = 0;
};

#endif

