#include "moduleManagers.h"
#include "ModuleManager.h"

#include "debugging/logging/logPlayer.h"

#include <vector>
#include <thread>


/*------------------------------------------------------------------------------------------------*/

/** Constructor
 **
 ** @param allEnabled   Whether all managers should be enabled initially
 */

ModuleManagersState::ModuleManagersState(bool allEnabled)
	: areManagersOnByDefault(allEnabled)
{
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor
 */

ModuleManagersState::~ModuleManagersState() {
}


/*------------------------------------------------------------------------------------------------*/

/** Get the run level for the given manager
 **
 ** @param name  Type name of manager
 ** @return run level if set, otherwise 0 (when off) or 1 (when on by default)
 */

int ModuleManagersState::getLevel(TypeIDName name) const {
	const auto &it = levels.find(name);
	if (it == levels.end()) {
		return areManagersOnByDefault ? 1 : 0;
	} else
		return it->second;
}


/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------------------------*/

/** Constructor
 **
 */

ModuleManagers::ModuleManagers()
{
	cs.setName("ModuleManagers");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

ModuleManagers::~ModuleManagers() {
	destroyManagers();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ModuleManagers::setActiveModules(const Config &config) {
	for (const auto& it : instances) {
		it.second->setActiveModules(config);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

/// start the module managers
void ModuleManagers::startManagers(const ModuleManagersState *state) {
	CriticalSectionLock lock(cs);

	if (LogPlayer::isActive()) {
		assert(nullptr == logPlayer);
		logPlayer = new LogPlayer();

		for (auto &instance : instances) {
			int runLevel = state->getLevel(instance.first);
			if (runLevel > 0 && logPlayer->isActive(instance.second->getName())) {
				logPlayer->start(instance.second);
			}
		}
	} else {
		for (auto &instance : instances) {
			int runLevel = state->getLevel(instance.first);
			if (runLevel > 0) {
				threads[instance.first] = std::thread([&](){ instance.second->startManager(runLevel); });
			}
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

/// delete the module managers
void ModuleManagers::destroyManagers() {
	CriticalSectionLock lock(cs);

	for (auto &it : instances) {
		// only stop/destroy managers that are actually running
		if (threads.count(it.first) == 0)
			continue;

		// issue a stop signal to the manager
		if (nullptr == logPlayer)
			it.second->stopManager();
		else
			logPlayer->cancel(true);

		// wait for the manager to stop
		threads[it.first].join();

		// destropy the manager
		delete it.second;
		it.second = nullptr;
	}

	threads.clear();
	instances.clear();
}
