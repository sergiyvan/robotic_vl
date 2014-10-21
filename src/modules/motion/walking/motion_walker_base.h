/** @file
 **
 */

#ifndef __MOTION_WALKER_ABSTRACTION__
#define __MOTION_WALKER_ABSTRACTION__

#include <list>
#include <queue>


/*------------------------------------------------------------------------------------------------*/

/**
 ** Enumeration of the feet
 */

enum Foot {
	  NO_FOOT                     = 0
	, LEFT_FOOT                   = 1
	, RIGHT_FOOT                  = 2
};


/*------------------------------------------------------------------------------------------------*/

/**
 ** Step represents a planned step.
 */

struct Step {
	int x;
	int y;
	int yaw;

	Step(int x, int y, int yaw)
		: x(x)
		, y(y)
		, yaw(yaw)
	{}
};


/*------------------------------------------------------------------------------------------------*/

/**
 ** Base class ("interface") for walking.
 */

class MotionWalker {
protected:
	/// which foot to kick with
	static Foot kick;

	/// whether foot step planning is activated
	static bool footPlanningMode;

	/// steps
	static std::queue<Step> steps;


public:
	virtual ~MotionWalker() {}

	/*** de/activate foot step planning mode */
	static void setFootPlanningMode(bool onoff) {
		footPlanningMode = onoff;
	}

	/*** query whether foot step planning mode is activated */
	static bool isFootPlanningMode() {
		return footPlanningMode;
	}

	/*** set the next step position ***/
	static void setNextStep(int x, int y, int yaw);

	/*** Kick with the right leg in next step */
	static void kickRight() { if (kick == NO_FOOT) kick = RIGHT_FOOT; }

	/*** Kick with the left leg in the next step */
	static void kickLeft()  { if (kick == NO_FOOT) kick = LEFT_FOOT;  }
};

#endif
