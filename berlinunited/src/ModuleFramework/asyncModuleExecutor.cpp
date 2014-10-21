#include "asyncModuleExecutor.h"

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void AsyncModuleExecutor::threadMain() {
	while (isRunning()) {
		if (startModuleEvent.wait(100*milliseconds)) {
			module->execute();
			startModuleEvent.reset();
			moduleFinishedEvent.trigger();
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

AsyncModuleExecutor::AsyncModuleExecutor() {
	cs.setName("AsyncModuleExecutor");
	moduleFinishedEvent.trigger();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

AsyncModuleExecutor::~AsyncModuleExecutor() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void AsyncModuleExecutor::executeModule(AbstractModuleCreator* module) {
	// wait for any executing module to finish
	waitForModuleToFinish();

	// clear flag
	moduleFinishedEvent.reset();

	// store pointer (safe operation as thread is waiting for trigger)
	this->module = module;

	// trigger execution
	startModuleEvent.trigger();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @return true if last executed module has finished, false on timeout
 */
bool AsyncModuleExecutor::waitForModuleToFinish(Millisecond timeout) {
	return moduleFinishedEvent.wait(timeout);
}
