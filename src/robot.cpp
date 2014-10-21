/** @file
 **
 ** Implementation of the Robot class which represents this robot.
 **
 */

#include "management/commandLine.h"
#include "management/config/config.h"

#include "debug.h"
#include "services.h"

#include "modules/motion/motion.h"

#include <fstream>
#include <sstream>


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto switchMinimal = ConfigRegistry::getInstance().registerSwitch("minimal",  "Only initalize the robot minimally (e.g. no vision, ...");
	auto switchNoIdle  = ConfigRegistry::getInstance().registerSwitch("noidle",   "Do not go into idle by default");
}


/*------------------------------------------------------------------------------------------------*/

class RobotCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		std::string commandName = cmdLine.getCommand(0);

		// the idle motion is played automatically at startup, so this
		// is just a dummy routine
		if (commandName == "idle") {
			ModuleManagers::getInstance().startManagers(cmdLine.getActiveCommand().moduleManagersState);
			sleep(1); // wait a bit for the idle motion to finish // TODO: very special magic number
		}

		return true;
	}
};

namespace {
	auto cmdIdle = CommandLine::getInstance().registerCommand<RobotCmdLineCallback>(
			"idle",
			"Puts robot into idle",
			ModuleManagers::none()->enable<Motion>());

	auto cmdInfo = CommandLine::getInstance().registerCommand<RobotCmdLineCallback>(
			"info",
			"Print general information about the robot status",
			ModuleManagers::none());
}
