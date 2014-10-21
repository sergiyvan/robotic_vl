#ifndef MODULEMANAGERS_H_
#define MODULEMANAGERS_H_

#include "utils/patterns/factory.h"
#include "utils/patterns/singleton.h"
#include "management/config/config.h"

class ModuleManager;

#include <vector>
#include <string>
#include <typeinfo>

/*------------------------------------------------------------------------------------------------*/

class LogPlayer;


/*------------------------------------------------------------------------------------------------*/

/**
 ** @ingroup ModuleFramework
 */

class ModuleManagersState {
private:
	typedef std::string TypeIDName;

public:
	explicit ModuleManagersState(bool allEnabled);
	~ModuleManagersState();

	/// disable a specific module manager
	template<class T>
	ModuleManagersState* disable() {
		return enable<T>(0);
	}

	/// enable a specific module manager
	template<class T>
	ModuleManagersState* enable(int level=1) {
		TypeIDName name = typeid(T).name();
		levels[name] = level;
		return this;
	}

	int getLevel(TypeIDName name) const;

private:
	bool areManagersOnByDefault;

	std::map<TypeIDName, int> levels;
};


/*------------------------------------------------------------------------------------------------*/

/** @class ModuleManagers
 **
 ** Responsible for instantiating and starting the registered module managers,
 ** based on an internal configuration.
 */


class ModuleManagers
	: public Singleton<ModuleManagers>
{
	friend class Singleton<ModuleManagers>;

protected:
	ModuleManagers();

public:
	~ModuleManagers();

	/// create a state instance with no enabled module manager by default
	static ModuleManagersState* none() { return new ModuleManagersState(false); }

	/// create a state instance with all module managers enabled by default
	static ModuleManagersState* all() { return new ModuleManagersState(true); }

	/// configure the active modules based on the configuration
	void setActiveModules(const Config &config);

	/// start the module managers
	void startManagers(const ModuleManagersState *state);

	/// delete the module managers
	void destroyManagers();

	/// get a specific manager
	template<class T>
	T* get() {
		CriticalSectionLock lock(cs);

		TypeIDName name = typeid(T).name();
		if (instances.find(name) == instances.end())
			instances[name] = new T();

		return dynamic_cast<T*>(instances[name]);
	}


protected:
	CriticalSection cs;

	LogPlayer *logPlayer;

	typedef std::string TypeIDName;
	std::map<TypeIDName, ModuleManager*> instances;
	std::map<TypeIDName, std::thread>    threads;
};

#endif
