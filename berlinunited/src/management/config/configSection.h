/*
 * configSection.h
 *
 *  Created on: Jul 27, 2014
 *      Author: dseifert
 */

#ifndef CONFIGREGISTRY_H__
#include "configRegistry.h"
#endif

#ifndef CONFIGSECTION_H_
#define CONFIGSECTION_H_

#include "configRegistry.h"
#include "configOption.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

class ConfigSection {
public:
	ConfigSection(const std::string &sectionName)
		: sectionName(sectionName)
		, lockCount(0)
	{
		assert(sectionName != "" && sectionName != ".");
	}

	ConfigSection(const ConfigSection& other) = delete;
	ConfigSection& operator=(const ConfigSection& other) = delete;

	/**
	 **
	 ** @return name of the section
	 */
	const std::string &getName() const {
		return sectionName;
	}

	void addChild(std::shared_ptr<ConfigOptionInterface> child) {
		options[child->getName()] = child;
	}

	/**
	 **
	 ** @param name
	 ** @param defaultValue
	 ** @param description
	 ** @return
	 */
	template <typename T>
	std::shared_ptr<ConfigOption<T>> registerOption(const std::string name, const T& defaultValue, const std::string &description) {
		return ConfigRegistry::getInstance().registerOption<T>(Config::getFullOptionName(sectionName, name), defaultValue, description);
	}

	/** Get access to an option by name.
	 **
	 ** @param name  Name of option within this section
	 **
	 ** @return a shared pointer to the config option (base class)
	 */

	std::shared_ptr<ConfigOptionInterface> getOption(const std::string name) {
		std::string fullName = Config::getFullOptionName(sectionName, name);

		for (auto it : options) {
			if (fullName == it.second->getName()) {
				return it.second;
			}
		}

		throw std::runtime_error("Option " + fullName + " does not exist.");
	}

	/** Get access to an option by name.
	 **
	 ** @param name  Name of option within this section
	 **
	 ** @return shared pointer to config option
	 */
	template <typename T>
	std::shared_ptr<ConfigOption<T>> getOption(const std::string name) {
		std::string fullName = Config::getFullOptionName(sectionName, name);

		for (auto &it : options) {
			if (fullName == it.second->getName()) {
				std::shared_ptr<ConfigOption<T>> option = std::dynamic_pointer_cast<ConfigOption<T>>(it.second);;

				if (nullptr != option) {
					return option;
				} else {
					std::string msg = "Accessing option " + fullName + " (of type " + it.second->getTypeName() + ") with wrong type";
					throw std::runtime_error(msg);
				}
			}
		}

		throw std::runtime_error("Option " + fullName + " does not exist.");
	}

	/** Lock the section and all options within it.
	 */
	void lock() {
		CriticalSectionLock lock(cs);

		++lockCount;
		for (auto &it : options) {
			it.second->lock();
		}
	}

	/** Unlock the section and all options within it.
	 */
	void unlock() {
		CriticalSectionLock lock(cs);

		if (lockCount > 0) {
			--lockCount;

			for (auto &it : options) {
				it.second->unlock();
			}
		}
	}

private:
	CriticalSection cs;

	std::string sectionName;
	uint32_t lockCount;

	std::map<std::string, std::shared_ptr<ConfigOptionInterface>> options;
};


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

class ConfigSectionLock {
protected:
	std::shared_ptr<ConfigSection> section;

public:
	ConfigSectionLock(std::shared_ptr<ConfigSection> sectionToLock)
		: section(sectionToLock)
	{
		if (nullptr != section)
			section->lock();
	}

	~ConfigSectionLock() {
		if (nullptr != section)
			section->unlock();
	}
};


#endif /* CONFIGSECTION_H_ */
