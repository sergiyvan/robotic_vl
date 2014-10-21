/** @file
 ** watchDog.cpp
 **
 **  Created on: May 19, 2009
 **      Author: dseifert
 */

#include "watchDog.h"
#include "debug.h"
#include "services.h"
#include "management/config/configRegistry.h"
#include "management/config/config.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>

namespace {
	auto cfgInterval = ConfigRegistry::registerOption<Second>("watchdog.interval", 3*seconds, "Watchdog timeout in seconds");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */

WatchDog::WatchDog()
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
 */

WatchDog::~WatchDog() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Watchdog thread.
 */

void WatchDog::threadMain() {
	// get interval (on 0, we don't run the watchdog)
	if (cfgInterval->get() < 1*seconds) {
		WARNING("Watchdog interval too small, disabling watchdog. Ignore if you set this on purpose.");
		return;
	}

	// open watchdog device file
	int fd = open("/dev/watchdog", O_WRONLY);
	if (fd < 0) {
		ERROR("Could not open watchdog device file");
		exit(-1);
	}

	int enable_flag = WDIOS_ENABLECARD;
	ioctl(fd, WDIOC_SETOPTIONS, &enable_flag);

	int intervalRounded = (int)ceil(cfgInterval->get().value());
	ioctl(fd, WDIOC_SETTIMEOUT, &intervalRounded);
	INFO("Set watchdog timeout to %d seconds", intervalRounded);

	// set us to lower niceness to make sure that we are not starved
	// out in a critical moment and fail to reset the watchdog in time
	setNiceness(-10);

	// sleep for close to the interval expires and then poke the watchdog
	__useconds_t sleepTime = 500000; // interval*1000000 - 250000;
	while (isRunning()) {
		int32_t flags = 1;
		ioctl(fd, WDIOC_KEEPALIVE, &flags);
		usleep(sleepTime);
	}

	close(fd);
}
