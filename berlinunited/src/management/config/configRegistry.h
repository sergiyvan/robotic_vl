#ifndef __CONFIG_H__
#include "config.h"
#endif

#ifndef CONFIGREGISTRY_H__
#define CONFIGREGISTRY_H__

#include "config.h"

class ConfigSection;

/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @ingroup config
 */

class ConfigRegistry
	: protected Config
	, public Singleton<ConfigRegistry>
{
	friend class Singleton<ConfigRegistry>;

private:
	ConfigRegistry()
		: registryClosed(false)
	{}

	bool registryClosed;

public:
	virtual ~ConfigRegistry() {}

	static std::shared_ptr<ConfigSection> getSection(const std::string &name) {
		assert(false == ConfigRegistry::getInstance().registryClosed);
		if (ConfigRegistry::getInstance().registryClosed)
			return nullptr;
		else
			return ((Config&)ConfigRegistry::getInstance()).getSection(name);
	}

	template <typename T>
	static std::shared_ptr<ConfigOption<T>> registerOption(std::string name, const T& defaultValue, const std::string &description) {
		assert(false == ConfigRegistry::getInstance().registryClosed);
		if (ConfigRegistry::getInstance().registryClosed)
			return nullptr;
		else
			return ((Config&)ConfigRegistry::getInstance()).registerOption(name, defaultValue, description);
	}

	// Register a switch
	bool registerSwitch(const std::string name, const std::string &description);

	// print the registered switches
	void printSwitches();

	// check whether the switch exists
	bool isSwitch(std::string name) const;

	// check whether the switch is set to true
	void copyTo(Config& config) {
		registryClosed = true;
		config = *this;
	}

	std::map< std::string, std::string > switches;
};

#endif
