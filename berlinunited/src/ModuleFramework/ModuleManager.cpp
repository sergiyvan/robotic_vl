#include "ModuleFramework/ModuleManager.h"
#include "ModuleFramework/Module.h"
#include "DataHolder.h"
#include "ModuleCreator.h"

#include "management/commandLine.h"

#include "debug.h"
#include "debugging/stopwatch.h"

#include "utils/utils.h"

#include "Serializer.h"
#include "debugging/logging/logWriter.h"

#include <map>
#include <set>
#include <string>
#include <list>
#include <iterator>
#include <vector>
#include <limits>
#include <ctime>
#include <cstdlib>
#include <iterator>
#include <algorithm>

/*------------------------------------------------------------------------------------------------*/

REGISTER_DEBUG("modules.showExecutionList", TEXT, CMDLOUT);
REGISTER_DEBUG("modules.showActiveModules", TEXT, CMDLOUT);

namespace {
	auto switchShow = ConfigRegistry::getInstance().registerSwitch("showmodules", "Show calculated execution list");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

ModuleManager::ModuleManager()
	: logWriter(nullptr)
	, runLevel(-1)
	, framenumber(0)
{
	executorCS.setName("ModuleManager::executorCS");
	startCS.setName("ModuleManager::startCS");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

ModuleManager::~ModuleManager() {
	CriticalSectionLock lock(executorCS);

	std::map<std::string, AbstractModuleCreator*>::iterator iter;
	for (iter = moduleExecutionMap.begin(); iter != moduleExecutionMap.end(); iter++) {
		delete (iter->second);
	}
}


/*------------------------------------------------------------------------------------------------*/

void ModuleManager::startManager(int level) {
	assert(nullptr == logWriter);

	CriticalSectionLock lock(startCS);

	runLevel = level;

	// generate the execution list
	calculateExecutionList();

	// create logger if requested
	if (LogWriter::isConfigured(getName())) {
		logWriter = new LogWriter(this);
		logWriter->start();
	}

	// print the execution list if requested
	if (CommandLine::getInstance().isSwitchEnabled("showmodules"))
		printExecutionList();

	if (false == executor.isRunning())
		executor.run();
	else {
		ERROR("%s's ASyncModuleExecutor is already running.", getName());
	}
}


/*------------------------------------------------------------------------------------------------*/

void ModuleManager::stopManager() {
	CriticalSectionLock lock(startCS);

	executor.cancel();
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** @brief Execute all active modules.
 */

void ModuleManager::executeModules() {
	CriticalSectionLock lock(executorCS);

	++framenumber;

	std::string runtimesStr = std::string(getName()) + ".runtimes";
	std::transform(runtimesStr.begin(), runtimesStr.end(), runtimesStr.begin(), ::tolower);
	DebuggingOption *debugOption = ::Debugging::getInstance().getDebugOption(runtimesStr);

	for (const auto& name : getExecutionList()) {
		// get entry
		AbstractModuleCreator* module = getModule(name);
		if (module != NULL && module->isEnabled()) {
			robottime_t startTime = getCurrentTime();

			bool stopWatchEnabled = (debugOption && debugOption->enabled);
			StopwatchItem& stopwatch = Stopwatch::getInstance().getStopwatchReference(runtimesStr, name);
			if (stopWatchEnabled)
				Stopwatch::getInstance().notifyStart(stopwatch, runtimesStr, name);

			// execute asynchronously
			executor.executeModule(module);
			Millisecond lastOutput = startTime;
			while (! executor.waitForModuleToFinish(1000*milliseconds)) {
				if (getCurrentTime() - startTime > 1000*milliseconds) {
					ERROR("%s module %s has been running for longer than %.1f seconds", getName(), name.c_str(), Second(getCurrentTime() - startTime).value());
					lastOutput = getCurrentTime();
				}
			}

			if (stopWatchEnabled)
				Stopwatch::getInstance().notifyStop(stopwatch, runtimesStr, name);
		}
	}

	if (debugOption && debugOption->enabled)
		Stopwatch::getInstance().send(runtimesStr);

	// write log file if available
	if (logWriter) {
		logWriter->serialize(framenumber, getBlackBoard().getRegistry());
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Enables/disables the modules based on the provided configuration
 **
 */

void ModuleManager::setActiveModules(const Config &config) {
	// use the configuration in order to set whether a module is activated or not
	for (std::list<std::string>::const_iterator name = getExecutionList().begin(); name != getExecutionList().end(); name++) {
		bool active = false;
		std::string key = std::string("modules." ) + getName() + "." + *name;

		if (false == config.exists(key))
			ERROR("Key %s not found", key.c_str())
		else if (config.get<bool>(key)) {
			active = true;
		}

		DEBUG_TEXT("modules.showActiveModules", "Activating %s module %s: %s", getName(), (*name).c_str(), active ? "yes" : "no");
		setModuleEnabled(*name, active);
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Enables/disables the module 'moduleName' (if it is registered).
 **
 ** @param moduleName                  Name of the module (as registered)
 ** @param value                       true to enable, false to disable module
 ** @param recalculateExecutionList    whether to recalculate the execution list
 */

void ModuleManager::setModuleEnabled(std::string moduleName, bool value, bool recalculateExecutionList) {
	if (moduleExecutionMap.find(moduleName) != moduleExecutionMap.end()) {
		moduleExecutionMap[moduleName]->setEnabled(value);
		if (recalculateExecutionList) {
			calculateExecutionList();
		}
	} else {
		std::cerr << (value ? "Enabling" : "Disabling") << "module ";
		std::cerr << moduleName << " failed as it was not found!\n";
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

AbstractModuleCreator* ModuleManager::getModule(const std::string& name) {
	std::map<std::string, AbstractModuleCreator*>::iterator iter = moduleExecutionMap.find(name);
	if (iter != moduleExecutionMap.end()) {
		return iter->second;
	}

	// TODO: assert?
	return nullptr;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

const AbstractModuleCreator* ModuleManager::getModule(const std::string& name) const {
	std::map<std::string, AbstractModuleCreator*>::const_iterator iter = moduleExecutionMap.find(name);
	if (iter != moduleExecutionMap.end()) {
		return iter->second;
	}

	// TODO: assert?
	return NULL;
}


/*------------------------------------------------------------------------------------------------*/

/** Calculate the execution list automatically from the dependencies.
 **
 */

void ModuleManager::calculateExecutionList() {
	CriticalSectionLock lock1(startCS);
	CriticalSectionLock lock2(executorCS);

	bool changed = true;
	std::list<std::string>::iterator start = moduleExecutionList.begin();
	const int maxAttempts = moduleExecutionList.size() * 10;
	int iterations = 0;

	while (changed && iterations < maxAttempts) {
		changed = false;
		iterations++;

		for (auto it1 = start; it1 != moduleExecutionList.end(); it1++) {
			start = it1;
			if (!moduleExecutionMap[*it1]->isEnabled())
				continue;
			Module* m1 = moduleExecutionMap[*it1]->getModule();

			for (auto it2 = it1; it2 != moduleExecutionList.end(); it2++) {
				if (!moduleExecutionMap[*it2]->isEnabled())
					continue;
				Module* m2 = moduleExecutionMap[*it2]->getModule();

				// m2 is currently executed after m1. Make sure that m2 does not provide a value
				// that m1 requires.
				for (auto itReq = m1->getRequiredRepresentations().begin();
				     itReq != m1->getRequiredRepresentations().end();
				     itReq++)
				{
					std::string repName = (*itReq)->getName();
					for (auto r = m2->getProvidedRepresentations().begin();
					     r != m2->getProvidedRepresentations().end();
					     ++r)
					{
						if ((*r)->getName() == repName) {
							std::swap(*it1, *it2);
							changed = true;
							break;
						}
					} //end for
					if (changed)
						break;
				} //end for
				if (changed)
					break;

				// m2 is currently executed after m1. Make sure that m1 does not provide a value
				// that m2 recycles.
				for (auto itProv = m1->getProvidedRepresentations().begin();
				     itProv != m1->getProvidedRepresentations().end();
				     itProv++)
				{
					std::string repName = (*itProv)->getName();
					for (auto r = m2->getRecycledRepresentations().begin();
					     r != m2->getRecycledRepresentations().end();
					     ++r)
					{
						if ((*r)->getName() == repName) {
							std::swap(*it1, *it2);
							changed = true;
							break;
						}
					} //end for
					if (changed)
						break;
				} //end for
				if (changed)
					break;
			} //end for
			if (changed)
				break;
		} //end for

	} //end while

	if (iterations >= maxAttempts) {
		WARNING("Maximal number of iterations reached for module execution list calculation");
	} //end if

	// check that all REQUIRE have a PROVIDE somewhere
	for (auto it1 = moduleExecutionList.begin();
	     it1 != moduleExecutionList.end();
	     it1++)
	{
		if (!moduleExecutionMap[*it1]->isEnabled())
			continue;
		Module* m1 = moduleExecutionMap[*it1]->getModule();

		for (auto itReq = m1->getRequiredRepresentations().begin();
		     itReq != m1->getRequiredRepresentations().end();
		     itReq++)
		{
			bool requirementMet = false;
			std::string repName = (*itReq)->getName();

			for (auto it2 = moduleExecutionList.begin();
			     it2 != it1 && !requirementMet;
			     it2++)
			{
				if (!moduleExecutionMap[*it2]->isEnabled())
					continue;
				Module* m2 = moduleExecutionMap[*it2]->getModule();
				for (auto r = m2->getProvidedRepresentations().begin();
				     r != m2->getProvidedRepresentations().end();
				     ++r)
				{
					if ((*r)->getName() == repName)
						requirementMet = true;
				}
			}

			if (!requirementMet)
				ERROR("Module %s's REQUIRE of %s is not met!", m1->getModuleName().c_str(), repName.c_str());
		}
	}

	// print execution list
	DEBUG_TEXT("modules.showExecutionList",
			"Calculated Module Execution List:");
	for (auto itExec = moduleExecutionList.begin();
	     itExec != moduleExecutionList.end();
	     itExec++)
	{
		if (moduleExecutionMap[*itExec]->isEnabled()) {
			DEBUG_TEXT("modules.showExecutionList", "  %s", (*itExec).c_str());
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Prints the list of execution order.
 **
 ** @param managerName  Name of module manager (for output purposes only)
 */

void ModuleManager::printExecutionList() {
	printf("Calculated execution list for '%s':\n", getName());
	for (auto itExec = moduleExecutionList.begin();
	     itExec != moduleExecutionList.end();
	     itExec++)
	{
		if (((AbstractModuleCreator*) getModule(*itExec))->isEnabled()) {
			printf("  %s\n", (*itExec).c_str());
		}
	}
}

/*------------------------------------------------------------------------------------------------*/

/** Given a list of representation names, disable all modules that provide
 ** any of these representations. This could be used to replace a set of modules
 ** providing data with custom modules (e.g. for simulators).
 **
 ** @param blacklisted  List of representation names
 */

void ModuleManager::deactivateModulesProviding(const std::list<std::string> &blacklisted) {
	for (auto moduleIter = moduleExecutionList.begin();
	     moduleIter != moduleExecutionList.end();
	     moduleIter++)
	{
		if (!moduleExecutionMap[*moduleIter]->isEnabled())
			continue;

		Module* module = moduleExecutionMap[*moduleIter]->getModule();

		// check provided representations
		for (auto providedIter = module->getProvidedRepresentations().begin();
		     providedIter != module->getProvidedRepresentations().end();
		     providedIter++)
		{
			std::string representationName = (*providedIter)->getName();
			for (auto blacklistedIter = blacklisted.begin();
			     blacklistedIter != blacklisted.end();
			     blacklistedIter++)
			{
				if (representationName == *blacklistedIter) {
					moduleExecutionMap[*moduleIter]->setEnabled(false);
					printf(
							"Disabled module %s because it provided representation %s\n",
							module->getModuleName().c_str(),
							representationName.c_str());
					break;
				}
			}
		}
	}
}
