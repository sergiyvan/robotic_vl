/*
 * Kinematic.cpp
 *
 *  Created on: Mar 15, 2011
 *      Author: johannes
 */

#include "kinematic.h"

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Kinematic::Kinematic() {
}
std::map<MotorID, Degree> Kinematic::setEndEffectors(const EndEffectorPose& pos_left, const EndEffectorPose& pos_right) const {
	std::map<MotorID, Degree> goal = inverseLegKinematics(pos_left, EndEffector::LEFT_LEG);
	std::map<MotorID, Degree> temp = inverseLegKinematics(pos_right, EndEffector::RIGHT_LEG);
	goal.insert(temp.begin(), temp.end());
	return goal;
}

