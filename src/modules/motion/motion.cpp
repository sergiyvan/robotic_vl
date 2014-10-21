#include "motion.h"
#include "services.h"

#include "platform/system/timer.h"
#include "management/commandLine.h"

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"
#include "platform/hardware/actuators/actuators.h"
#include "management/config/config.h"

#include "representations/motion/motionStatus.h"
#include "representations/motion/activeMotion.h"

#include <string>

#include "debug.h"
#include "debugging/stopwatch.h"


/*------------------------------------------------------------------------------------------------*/

REGISTER_DEBUG("motion.runtimes", STOPWATCH, BASIC);

namespace {
	auto cfgFPS = ConfigRegistry::registerOption<Hertz>("motion.fps", 100*hertz, "Number of iterations/s the motion layer should attempt to run");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

class MotionCommandLineInterface : public CommandLineInterface {
public:
	virtual ~MotionCommandLineInterface() {}

	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		std::string cmd = cmdLine.getCommand(0);

		if (cmd == "off") {
			RobotModel* model = RobotModel::createRobotModel();
			model->init();

			std::map<MotorID, bool> torque;
			for (auto id : model->getRobotDescription()->getMotorIDs()) {
				torque[id] = false;
			}
			model->getActuators()->setTorqueEnabled(torque);
		}

		return true;
	}
};

namespace {
	auto cmdOff = CommandLine::getInstance().registerCommand<MotionCommandLineInterface>(
			"off",
			"Turn off all motors",
			ModuleManagers::none());
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Motion::Motion() {
	startCS.setName("Motion");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Motion::~Motion() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Initialize the motion module block and activate the modules.
 */

void Motion::startManager(int level) {
	CriticalSectionLock lock(startCS);

	if (false == services.getRobotModel().isHardwareInitialized()) {
		ERROR("Robot model could not initialize hardware. Disabling motion layer.");
		return;
	}

	services.getEvents().trigger(EVT_MOTION_MODULES_READY, NULL);

	// set initial motion request if required
	if (level >= 2) {
		MotionStatus& motionStatus = *getBlackBoard().getRepresentation< DataHolder<MotionStatus> >("MotionStatus");
		motionStatus.motionHasFinished = false;
	}

	run();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Motion::stopManager() {
	CriticalSectionLock lock(startCS);
	cancel();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Motion::threadMain() {
	// initialize our thread
	executor.setRealTimePriority();
	ModuleManager::startManager(1);

	Microsecond nextTriggerTime = getCurrentMicroTime();
	const Hertz targetFPS = cfgFPS->get();
	const Microsecond interval = Microsecond(1./targetFPS);

	robottime_t abortRequestedTimestamp = 0*milliseconds;

	while (true) {
		// if we were requested to stop, try to make sure that the robot is
		// in a stable state
		if (!isRunning()) {
			MotionStatus& motionStatus = *getBlackBoard().getRepresentation< DataHolder<MotionStatus> >("MotionStatus");
			ActiveMotion& activeMotion = *getBlackBoard().getRepresentation< DataHolder<ActiveMotion> >("ActiveMotion");

			// if abort was requested just now ...
			if (abortRequestedTimestamp == 0*milliseconds) {
				abortRequestedTimestamp = getCurrentTime();

				// ignore any cognition input from now on
				motionStatus.cognitionInputEnabled = false;
			}

			// check whether we can safely abort
			if (motionStatus.robotIsStable == true || activeMotion.motion == MOTION_NONE)
				break;

			// abort after some time anyway
			if (abortRequestedTimestamp + Millisecond(3*seconds) < getCurrentTime())
				break;
		}

		// if our time is not up yet, sleep until it is
		Microsecond delayDuration = nextTriggerTime - getCurrentMicroTime();
		if (delayDuration > 0*microseconds)
			delay(delayDuration);

		nextTriggerTime = getCurrentMicroTime() + interval;

		/*======================*/
		// execute all modules
		/*======================*/

		executeModules();
	}

	ModuleManager::stopManager();
}
