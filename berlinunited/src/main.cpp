#ifdef BERLINUNITEDSTANDALONE
#include <signal.h>

#include "debug.h"
#include "services.h"

#include <string>

#include "management/commandLine.h"

#include "management/config/config.h"
#include "management/config/configProtobuf.h"


/*------------------------------------------------------------------------------------------------*/


/**
 ** Entry point function of the application.
 **
 ** @param argc  Number of arguments
 ** @param argv  Array of arguments
 */

int main(int argc, char* argv[]) {
	// Setup configuration. This must be done as the very first step as most
	// of the remaining initialization in some way or another depends on
	// some configuration value.
	services.setupConfig<ConfigProtobuf>("config.pbc");

	// parse command line
	CommandLine &cmdLine = CommandLine::getInstance();
	if (false == cmdLine.process(argc, argv)) {
		printf("Aborted\n");
		return -1;
	}

	// apply command line option values to configuration
	CommandLine::getInstance().applyTo(services.getConfig());

	// get info about the command we are about to execute
	CommandLineCommand cmd;
	if (false == cmdLine.getCommandInfo(cmd)) {
		printf("Unknown command %s.\n", cmdLine.getCommand(0).c_str());
		exit(-1);
	}

	// execute command function
	cmd.callback->commandLineCallback(cmdLine);

	return 0;
}

#endif
