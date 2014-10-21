#include "moduleManagerTriggered.h"

#include "services.h"


/*------------------------------------------------------------------------------------------------*/

/** Constructor
 **
 ** @param eventType   Event type to trigger the execution
 */

ModuleManagerTriggered::ModuleManagerTriggered(EventType eventType)
	: eventType(eventType)
{
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor
 */

ModuleManagerTriggered::~ModuleManagerTriggered() {

}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ModuleManagerTriggered::startManager(int runlevel) {
	CriticalSectionLock lock(startCS);

	ModuleManager::startManager(runlevel);

	// register for event
	services.getEvents().registerForEvent(eventType, this);
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

void ModuleManagerTriggered::stopManager() {
	CriticalSectionLock lock(startCS);

	// unregister event that triggers restart first!
	services.getEvents().unregisterForEvent(EVT_IMAGE_CAPTURED, this);

	ModuleManager::stopManager();
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Execute all active modules when the event callback occurs.
 */

void ModuleManagerTriggered::eventCallback(EventType evtType, void* data) {
	if (evtType == this->eventType) {
		executeModules();
	}
}
