#include "moduleManagerClocked.h"

#include "services.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

ModuleManagerClocked::ModuleManagerClocked() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

ModuleManagerClocked::~ModuleManagerClocked() {

}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ModuleManagerClocked::startManager(int runLevel) {
	run();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ModuleManagerClocked::stopManager() {
	cancel();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ModuleManagerClocked::threadMain() {
	ModuleManager::startManager(1);

	auto cfgSection = services.getConfig().getSection(getName());
	const Hertz targetFPS = cfgSection->getOption<Hertz>("fps")->get();
	const robottime_t intervalInMs = Millisecond(1./targetFPS);
	robottime_t nextTriggerTime = getCurrentTime();

	while (isRunning()) {
		// if our time is not up yet, sleep until it is
		Millisecond delayDuration = nextTriggerTime - getCurrentTime();
		if (delayDuration > 0*milliseconds)
			delay(delayDuration);

		nextTriggerTime += intervalInMs;
		executeModules();
	}
}
