/**
 * @file
 *
 * This file can be used to add routines to test your new code.
 *
 * Please don't commit this file. The tests should stay on your computer.
 */

#include "debug.h"
#include "services.h"
#include "platform/hardware/robot/robotModel.h"

#include "communication/comm.h"
#include "management/commandLine.h"

// module framework
#include "modules/motion/motion.h"

// representations
#include "platform/hardware/robot/robotDescription.h"
#include "representations/motion/kinematicTree.h"
#include "representations/motion/kinematicengine/kinematicEngineTasks.h"
#include "tools/kinematicEngine/tasks/kinematicEngineTaskLocation.h"
#include "tools/kinematicEngine/tasks/kinematicEngineTaskOrientation.h"
#include "tools/kinematicEngine/tasks/kinematicEngineTaskCOM.h"
#include "tools/kinematicEngine/tasks/kinematicEngineTaskCOMWholeRobot.h"


#include <armadillo>

#include <sstream>
#include <iostream>


/*------------------------------------------------------------------------------------------------*/

BEGIN_DECLARE_MODULE(PlayCoordinateSystem)
	REQUIRE(KinematicTree)
	PROVIDE(KinematicEngineTasks)
END_DECLARE_MODULE(PlayCoordinateSystem)

class PlayCoordinateSystem : public PlayCoordinateSystemBase {
private:
	KinematicEngineTaskOrientation m_taskLeftHandDirectionX;
	KinematicEngineTaskOrientation m_taskRightHandDirectionX;
	KinematicEngineTaskOrientation m_taskCameraDirectionX;


public:

	virtual void init() {
		RobotDescription const& robotDescription = *services.getRobotModel().getRobotDescription();
		m_taskLeftHandDirectionX = KinematicEngineTaskOrientation("playCoordinateSystemX", robotDescription.getEffectorID("gyroscope"), robotDescription.getEffectorID("LeftHand"), getKinematicEngineTasks().getTree(), KinematicEngineTaskOrientation::AXIS_X);
		m_taskRightHandDirectionX = KinematicEngineTaskOrientation("playCoordinateSystemY", robotDescription.getEffectorID("gyroscope"), robotDescription.getEffectorID("RightHand"), getKinematicEngineTasks().getTree(), KinematicEngineTaskOrientation::AXIS_X);
		m_taskCameraDirectionX = KinematicEngineTaskOrientation("playCoordinateSystemZ", robotDescription.getEffectorID("gyroscope"), robotDescription.getEffectorID("CameraPosition"), getKinematicEngineTasks().getTree(), KinematicEngineTaskOrientation::AXIS_X);

		m_taskLeftHandDirectionX.setTarget(arma::colvec({0, 1, 0}));
		m_taskRightHandDirectionX.setTarget(arma::colvec({0, 0, 1}));
		m_taskCameraDirectionX.setTarget(arma::colvec({1, 0, 0}));
	}

	virtual void execute() {

		getKinematicEngineTasks().addTask(&m_taskLeftHandDirectionX);
		getKinematicEngineTasks().addTask(&m_taskRightHandDirectionX);
		getKinematicEngineTasks().addTask(&m_taskCameraDirectionX);
	}
};




/*------------------------------------------------------------------------------------------------*/

class PlayCoordCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// enable the test modules
		services.getModuleManagers().get<Motion>()->setModuleEnabled("PlayCoordinateSystem", true);

		// wait for termination - comment this line if you want to quit right away
		services.runManagers();
		return true;
	}
};

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion,    PlayCoordinateSystem,    false, "Robot pretends to be a coordinate system")

namespace {
	auto cmdPlay = CommandLine::getInstance().registerCommand<PlayCoordCmdLineCallback>(
			"playCoordinateSystem",
			"Robot play coordinate system",
			ModuleManagers::none()->enable<Motion>());
}
