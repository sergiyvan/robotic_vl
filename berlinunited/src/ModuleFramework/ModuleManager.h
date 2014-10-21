/**
 * @file ModuleManager.h
 *
 * @author <a href="mailto:mellmann@informatik.hu-berlin.de">Heinrich Mellmann</a>
 * Declaration of class ModuleManager (base class for ModuleManagers)
 */

#ifndef __MODULEMANAGER_H_
#define __MODULEMANAGER_H_

#include <map>
#include <string>
#include <list>
#include <vector>

#include "asyncModuleExecutor.h"
#include "BlackBoard.h"
#include "Module.h"
#include "ModuleCreator.h"
#include "utils/namedInstance.h"
#include "management/config/config.h"

/*------------------------------------------------------------------------------------------------*/

// forward declarations
class LogWriter;
class LogPlayer;


/*------------------------------------------------------------------------------------------------*/

/**
 ** @ingroup ModuleFramework
 */

class ModuleManager
		: virtual public BlackBoardInterface
		, virtual public NamedInstance
{
public:
	ModuleManager();
	virtual ~ModuleManager();

	/** Start the module manager with the runlevel (>= 1). Should execute
	 ** the necessary steps to have the modules executed (e.g. by starting
	 ** a thread with a loop, or event-based).
	 **
	 ** It is recommended to protect the code within this function and the
	 ** stopManager() call with a critical section, so a stop of the manager
	 ** during the start phase will wait for the start up to finish.
	 **
	 ** @param level   runlevel, is greater or equal to 1
	 */
	virtual void startManager(int level);

	/** Stop the module manager. Should abort the module execution and clean
	 ** up anything created/started in startManager().
	 **
	 ** It is recommended to protect the code within this function and the
	 ** startManager() call with a critical section, so a stop of the manager
	 ** during the start phase will wait for the start up to finish.
	 */
	virtual void stopManager();

	/** Register a module.
	 **
	 ** Registration of different instances of the same class with different names
	 ** is allowed
	 **
	 ** @param name      name of the module, it is strongly recommended
	 **                  recommended to have it identical to the class name.
	 ** @param enable    whether this module is enabled by default
	 **
	 ** @return a module creator instance
	 */
	template<class T>
	ModuleCreator<T>* registerModule(std::string name, bool enable = false) {
		// module does not exist
		if (moduleExecutionMap.find(name) == moduleExecutionMap.end()) {
			moduleExecutionList.push_back(name);
			moduleExecutionMap[name] = createModule<T>();
		}

		AbstractModuleCreator* module = moduleExecutionMap.find(name)->second;
		ModuleCreator<T>* typedModule = dynamic_cast<ModuleCreator<T>*>(module);

		// check the type
		if (typedModule == NULL) {
			std::cerr << "Module type mismatch: " << name
			          << " is already registered as "
			          // HACK: getModuleName doesn't necessary return the type of the module
			          << module->getModule()->getModuleName() << ", but "
			          << typeid(T).name() << " requested." << std::endl;
			assert(false);
		}

		typedModule->setEnabled(enable);

		return typedModule;
	}

	void deactivateModulesProviding(const std::list<std::string> &representations);

	void setActiveModules(const Config &config);

	void setModuleEnabled(std::string moduleName, bool value = true, bool recalculateExecutionList = false);

public: // TODO: make protected, at the moment required by SimStar support
	AbstractModuleCreator* getModule(const std::string& name);
	const AbstractModuleCreator* getModule(const std::string& name) const;

protected:
	friend class LogWriter;
	friend class LogPlayer;
	LogWriter *logWriter;

	const std::list<std::string>& getExecutionList() {
		return moduleExecutionList;
	}

	virtual void executeModules();

	void calculateExecutionList();
	void printExecutionList();

	// cached value of the runlevel
	int runLevel;

	// critical section for start/stop (i.e. we do not want to execute a shutdown
	// while still powering up)
	CriticalSection startCS;

	// critical section for the execution of modules
	CriticalSection executorCS;

	// asynchronous module executor instance
	AsyncModuleExecutor executor;

	// the number of iterations
	uint32_t framenumber;

private:
	template<class T>
	ModuleCreator<T>* createModule() {
		return new ModuleCreator<T>(getBlackBoard());
	}

	/** store the mapping name->module */
	std::map<std::string, AbstractModuleCreator*> moduleExecutionMap;

	/** list of names of modules in the order of execution */
	std::list<std::string> moduleExecutionList;
};

#endif //__ModuleManager_h_
