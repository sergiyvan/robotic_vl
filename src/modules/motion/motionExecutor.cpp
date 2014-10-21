#include "motionExecutor.h"
#include "modules/motion/motion.h"

//#include "static/motion_static.h"

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/power/power.h"
#include "platform/hardware/motions/motions.h"

#include "services.h"
#include "debug.h"
#include "management/config/config.h"


REGISTER_MODULE(Motion, MotionExecutor, true, "Trigger execution of correct motion");

namespace {
	auto cfgLocomotion = ConfigRegistry::registerOption<std::string>("motions.locomotion", "walker", "Locomotion mechanism: walker, threewheel");
}

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

MotionExecutor::MotionExecutor()
	: locomotionType(INVALID)
{
	walkerModule = registerModule<LearningWalker>(std::string("LearningWalker"));
	walkerModule->setEnabled(true);

//	staticMotionModule = registerModule<MotionStatic>(std::string("StaticMotion"));
//	staticMotionModule->setEnabled(true);
//
//	threeWheelModule = registerModule<ThreeWheel>(std::string("ThreeWheel"));
//	threeWheelModule->setEnabled(true);
//
//	fallingDownModule = registerModule<FallingDownMotion>(std::string("FallingDownMotion"));
//	fallingDownModule->setEnabled(true);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

MotionExecutor::~MotionExecutor() {
	services.getEvents().unregisterForEvent(EVT_CONFIGURATION_LOADED, this);
}


/*------------------------------------------------------------------------------------------------*/

/** Initialize the module. This is called by the framework when execute() is
 ** called for the first time.
 */

void MotionExecutor::init() {
	services.getEvents().registerForEvent(EVT_CONFIGURATION_LOADED, this);
	eventCallback(EVT_CONFIGURATION_LOADED, &services.getConfig());

	walkerModule->init();
//	threeWheelModule->init();
//	fallingDownModule->init();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void MotionExecutor::execute() {
	// check whether motors have power - if they don't, we abort all motions
	if (false == services.getRobotModel().getPower()->hasPowerToActuators()) {
		if (getActiveMotion().motion != MOTION_NONE) {
			WARNING("Killing active motion because power has been cut to servos.");
		}

		getMotionStatus().motionHasFinished = true;
		getMotionStatus().robotIsStable = false;
		return;
	}

	// the active motion has run at least once, so init should be over
	getActiveMotion().motionInit = false;

	// MOTION_STANDUP is a special case that maps to multiple
	// other motions
	if (getMotionRequest().motion == MOTION_STANDUP
	    && (    getActiveMotion().motion == MOTION_STANDUP_BACK
	         || getActiveMotion().motion == MOTION_STANDUP_FRONT
	       ))
	{
		// no motion change required

	} else if (getMotionRequest().motion != getActiveMotion().motion) {
		// motion change requested

		// check whether we can allow it
		if (getMotionStatus().robotIsStable || getMotionStatus().motionHasFinished) {
			getActiveMotion().motion = getMotionRequest().motion;
			getActiveMotion().motionInit        = true;
			getActiveMotion().stopWhenStable    = false;
			getMotionStatus().motionHasFinished = false;
		} else {
			// request the running motion to come to an end
			getActiveMotion().stopWhenStable = true;
		}
	}

	// execute motion
	if (services.getRobotModel().getMotions()->hasMotion( getActiveMotion().motion )) {
//		staticMotionModule->execute();

	} else {
		switch (getActiveMotion().motion) {
		// move the robot
		case MOTION_LOCOMOTION:
			switch (locomotionType) {
			case WALKER:
				walkerModule->execute();
//				fallingDownModule->execute();
				break;
			default:
				break;
			}
			break;

		// MOTION_STANDUP is a special case that may map to different motions
		case MOTION_STANDUP:
			standup();
			break;

		case MOTION_KICK:
			ERROR("Dynamic kick is not implemented");
			//dynKickModule->execute();
//			fallingDownModule->execute();
			break;

		case MOTION_NONE:
			// nothing to be done, relax and enjoy the sun
			break;

		default:
			//ERROR("Unknown motion requested: %d", getActiveMotion().motion);
			break;
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Handle standing up.
 */

void MotionExecutor::standup() {
//	switch (getRobotPosture().posture) {
//		case ROBOT_FALLEN_FORWARD:
//			getActiveMotion().motion = MOTION_STANDUP_FRONT;
//			break;
//
//		// when fallen on side, try to get into back or front position
//		case ROBOT_FALLEN_LEFTSIDE:
//			getActiveMotion().motion = MOTION_PREPARE_LEFT_STANDUP;
//			break;
//		case ROBOT_FALLEN_RIGHTSIDE:
//			getActiveMotion().motion = MOTION_PREPARE_RIGHT_STANDUP;
//			break;
//		case ROBOT_FALLEN_BACKWARD:
//			getActiveMotion().motion = MOTION_STANDUP_BACK;
//			break;
//
//		default:
//			ERROR("Standup motion requested but robot does not seem to have fallen down");
//			getMotionStatus().motionHasFinished = true;
//			break;
//	}
//
//	// execute motion once (initialisation was done in execute())
//	if (services.getRobotModel().getMotions()->hasMotion( getActiveMotion().motion ))
//		staticMotionModule->execute();

}


/*------------------------------------------------------------------------------------------------*/

void MotionExecutor::eventCallback(EventType eventType, void* data) {
	if (eventType == EVT_CONFIGURATION_LOADED) {
		std::string motionTypeStr = cfgLocomotion->get();
		if (motionTypeStr == "walker")
			locomotionType = WALKER;
//		else if (motionTypeStr == "threewheel")
//			locomotionType = THREEWHEEL;
		else
			ERROR("Unknown locomotion type %s, select one of walker or threewheel", motionTypeStr.c_str());
	}
}
