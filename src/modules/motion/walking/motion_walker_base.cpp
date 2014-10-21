/** @file
 **
 **
 */

#include "motion_walker_base.h"
#include "management/commandLine.h"
#include "management/config/configRegistry.h"
#include "debug.h"

#include <queue>


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgMaxFwd = ConfigRegistry::registerOption<int>("motions.walker.maxForwardSpeed",  20, "The maximum speed, which the robot can walk if he walks only forward.");
	auto cfgMaxBwd = ConfigRegistry::registerOption<int>("motions.walker.maxBackwardSpeed", 18, "The maximum speed, which the robot can walk if he walks only backward.");
	auto cfgMaxRot = ConfigRegistry::registerOption<int>("motions.walker.maxRotationSpeed",  9, "The maximum speed, which the robot can walk if he only rotates.");
	auto cfgMaxSwd = ConfigRegistry::registerOption<int>("motions.walker.maxSidewardSpeed", 18, "The maximum speed, which the robot can walk if he walks only side.");
}


/*------------------------------------------------------------------------------------------------*/

std::queue<Step> MotionWalker::steps;
bool MotionWalker::footPlanningMode = false;
Foot MotionWalker::kick = NO_FOOT;


/*------------------------------------------------------------------------------------------------*/

void MotionWalker::setNextStep(int x, int y, int yaw) {
	if(footPlanningMode) {
		steps.push(Step(x, y, yaw));
	} else {
		ERROR("Foot step planning mode is inactive but step is set.");
	}
}
