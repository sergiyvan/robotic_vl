#include "services.h"
#include "servicesInit.h"

#include "management/commandLine.h"
#include "management/config/config.h"
#include "platform/hardware/robot/robotModel.h"
#include "utils/stringTools.h"


// (required for SimStar)
DEFINE_EVENT_ID(EVT_MOTION_MODULES_READY, "Motions modules are set up and about to be executed for the first time");

namespace {
	auto switchSimstar  = ConfigRegistry::getInstance().registerSwitch("simstar",  "Use Sim* as simulator");
	auto switchSimspark = ConfigRegistry::getInstance().registerSwitch("simspark", "Use SimSpark as simulator");

	auto cfgDebug = ConfigRegistry::registerOption<std::string>("debug", "", "Comma-separated list (without spaces) of debug options to enable at start");
}


/*------------------------------------------------------------------------------------------------*/

Services &services = Services::getInstance();


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */

Services::Services()
	: robotModel(nullptr)
	
{
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor.
 **
 ** Deletes remaining resources.
 */

Services::~Services() {
	delete robotModel;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool Services::init(int argc, char* argv[], ServiceInitStructure* sis) {
	if (false == ServicesBase::init(argc, argv))
		return false;

	// setup robot model
	robotModel = RobotModel::createRobotModel();
	robotModel->init();

	// enable debug output for modules
	const std::string debugOptionsToEnable = getConfig().get<std::string>("debug");
	std::vector<std::string> optionNames;
	split(debugOptionsToEnable, ",", optionNames);
	for (auto optionName : optionNames) {
		DebuggingOption *debugOption = ::Debugging::getInstance().getDebugOption(optionName);
		if (debugOption) {
			debugOption->enabled = true;
		}
	}

	// actually register events (required for SimStar)
	REGISTER_EVENT_ID(EVT_MOTION_MODULES_READY);
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */
/*
Camera* Services::getSimulatorCamera() {
	if (simulator != nullptr)
		return simulator->getCameraInstance();
	else
		return nullptr;
}
*/
