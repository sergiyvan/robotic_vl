/** @file
 **
 ** Entry point for the FUmanoid application.
 **
**/

#include "services.h"
#include "management/config/configProtobuf.h"

// get build info, this is created when compiling
#define BUILDINFO "(\?\?\?)"
#include "buildInfo.h"

/*------------------------------------------------------------------------------------------------*/

/**
 ** Entry point function of the application.
 **
 ** @param argc  Number of arguments
 ** @param argv  Array of arguments
 */

int main(int argc, char* argv[]) {
	printf("FUmanoid starting - %s\n", BUILDINFO);

	// Setup configuration. This must be done as the very first step as most
	// of the remaining initialization in some way or another depend on
	// some configuration value.
	services.setupConfig<ConfigProtobuf>("config/FUmanoid.pbc");

	// Setup the framework, run all necessary initialization
	if (false == services.init(argc, argv)) {
		return -1;
	}

	// run
	return services.run();
}
