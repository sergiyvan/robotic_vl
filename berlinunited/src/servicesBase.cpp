/** @defgroup services
 **
 */

#include "debug.h"
#include "servicesBase.h"
#include "servicesInit.h"

#include "communication/comm.h"

#include "management/commandLine.h"
#include "management/config/config.h"
#include "management/config/configSection.h"

#include "platform/system/events.h"

#include "utils/keyboard.h"
#include "utils/signals.h"


class Services;
extern Services &services;


// definition and initialization of static member variables
Event ServicesBase::terminationEvent;

// TODO: re-define / move
DEFINE_EVENT_ID(EVT_CONFIGURATION_LOADED, "configuration has been loaded/updated");
DEFINE_EVENT_ID(EVT_BEFORE_CONFIG_SAVE,   "configuration is about to be saved");
DEFINE_EVENT_ID(EVT_IMAGE_CAPTURED,       "a new image was captured");


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgRobotSection = ConfigRegistry::getInstance().getSection("robot");
	auto cfgRobotID      = cfgRobotSection->registerOption<int>("id", -1, "ID of the robot");
	auto cfgRobotName    = cfgRobotSection->registerOption<std::string>("name", "", "Name of the robot");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */

ServicesBase::ServicesBase()
	: comm(nullptr)
	, config(nullptr)
	, events(nullptr)
	, messageRegistry()
	, moduleManagers(ModuleManagers::getInstance())
	, id(-1)
	, name("unnamed")
	, startTime(getCurrentTime())
{
	comm   = new Comm();
	events = new Events();

	REGISTER_BASE_EVENT_ID(EVT_CONFIGURATION_LOADED);
	REGISTER_BASE_EVENT_ID(EVT_BEFORE_CONFIG_SAVE);
	REGISTER_BASE_EVENT_ID(EVT_IMAGE_CAPTURED);
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor.
 **
 ** Deletes remaining resources.
 */

ServicesBase::~ServicesBase() {
	delete events;
	events = nullptr;

	delete comm;
	comm = nullptr;

	// unregister for configuration requests
	getMessageRegistry().unregisterMessageCallback(config, "configurationRequest");

	delete config;
	config = nullptr;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool ServicesBase::init(int argc, char* argv[], ServiceInitStructure* sis) {
	// check that the configuration object has been created
	if (nullptr == config) {
		ERROR("ServicesBase::init() failure as the configuration is not set up.");
		return false;
	}

	// set up signal handlers (handles Ctrl+C and crashes)
	setupSignals();

	// parse command line
	if (false == CommandLine::getInstance().process(argc, argv)) {
		printf("Aborted\n");
		return false;
	}

	// command line values have higher priority than the configuration file,
	// so we apply them now to the configuration
	CommandLine::getInstance().applyTo( getConfig() );

	// set robot ID and name
	id  = cfgRobotID->get();
	name= cfgRobotName->get();

	// assemble default robot name if no name is given
	if (name == "") {
		std::stringstream ss;
		ss << "robot" << id;
		name = ss.str();
	}

	// now init the services we offer
	comm->init();

	// init the status output dispatcher
	StatusOutput::getInstance().init();

	// activate modules based on the configuration
	moduleManagers.setActiveModules(getConfig());

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

int ServicesBase::run() {

	////////////////////////////////////////////////////////////////////////////
	// RUN
	////////////////////////////////////////////////////////////////////////////

	// get info about the command we are about to execute
	CommandLineCommand cmd;
	if (false == CommandLine::getInstance().getCommandInfo(cmd)) {
		printf("Unknown command %s.\n", CommandLine::getInstance().getCommand(0).c_str());
		return -1;
	}

	cmd.callback->commandLineCallback(CommandLine::getInstance());

	////////////////////////////////////////////////////////////////////////////
	// QUIT
	////////////////////////////////////////////////////////////////////////////

	ERROR("QUITTING (ignore any segfaults after this line)");

	terminate();
	releaseKeyboard();

	printf("Good bye.\n");
	return 0;
}


/*------------------------------------------------------------------------------------------------*/

/** Run the module managers and wait for their termination
 */

void ServicesBase::runManagers() {
	const ModuleManagersState *state = CommandLine::getInstance().getActiveCommand().moduleManagersState;

	if (state != nullptr)
		moduleManagers.startManagers(state);
	else {
		ERROR("No module manager configuration defined for active command");
	}

	waitForTermination();
}


/*------------------------------------------------------------------------------------------------*/

/** Run the module managers and wait for their termination
 */

void ServicesBase::runManagers(const ModuleManagersState *state) {
	services.getModuleManagers().startManagers(state);
	waitForTermination();
}


/*------------------------------------------------------------------------------------------------*/

/** Terminate the application.
 **
 ** @param notifyOfTermination  Whether to notify of termination
 */
void ServicesBase::terminate(bool notifyOfTermination) {
	INFO("Terminating application");

	// shutting down subsystems
	moduleManagers.destroyManagers();

	if (notifyOfTermination)
		triggerTermination();

	comm->cancel(true);

	// wait a little bit
	delay(500*milliseconds);
	printf("Termination sequence finished.\n");
}


/*------------------------------------------------------------------------------------------------*/

/** Trigger termination
 */

void ServicesBase::triggerTermination() {
	terminationEvent.trigger();
}


/*------------------------------------------------------------------------------------------------*/

/** Wait until robot is terminated
 */

void ServicesBase::waitForTermination() {
	terminationEvent.wait();
}
