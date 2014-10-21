#include "configRegistry.h"

#include <algorithm>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

#include <inttypes.h>
#include <assert.h>
#include <stdio.h>




/*------------------------------------------------------------------------------------------------*/

/** Register a pair of opposing switches.
 **
 ** @param name          Name of switch that is set by default (unless oppositeName is "")
 ** @param description   Description of switch
 **
 ** @return true if switches were successfully registered
 */

bool ConfigRegistry::registerSwitch(std::string name, const std::string &description) {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	// check whether an option of the same name exists
	bool optionExists = exists(name);
	assert(!optionExists);
	if (optionExists)
		return false;

	switches[name] = description;
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ConfigRegistry::printSwitches() {
	size_t maxLength = 15;
	for (auto it : switches)
		maxLength = std::max(maxLength, it.second.size());

	for (auto it : switches)
		printf("  %s%s- %s\n",
				it.first.c_str(),
				std::string(maxLength - it.first.size() + 1, ' ').c_str(),
				it.second.c_str());
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool ConfigRegistry::isSwitch(std::string switchName) const {
	std::transform(switchName.begin(), switchName.end(), switchName.begin(), ::tolower);
	return (0 != switches.count(switchName));
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

//void ConfigRegistry::applyDefaultValuesTo(Config *config) {
//	for (std::map<std::string, std::string>::iterator it = stringOptions.begin();
//	     it != stringOptions.end();
//	     ++it)
//	{
//		// Delete option if option exists as a different type
//		if (config->exists(it->first) && config->getOptionType("", it->first) != OPTIONTYPE_STRING) {
//			config->deleteConfigurationOption("", it->first);
//		}
//		// Reset default value if option exists, else create option
//		if (config->exists(it->first)) {
//			config->setDefaultValue(it->first, descriptions[it->first], it->second);
//		} else {
//			config->createConfigurationOption("", it->first, descriptions[it->first], OPTIONTYPE_STRING, it->second);
//		}
//	}
//
//	for (std::map<std::string, int>::iterator it = integerOptions.begin();
//	     it != integerOptions.end();
//	     ++it)
//	{
//		// Delete option if option exists as a different type
//		if (config->exists(it->first) && config->getOptionType("", it->first) != OPTIONTYPE_INTEGER) {
//			config->deleteConfigurationOption("", it->first);
//		}
//		// Reset default value if option exists, else create option
//		if (config->exists(it->first)) {
//			config->setDefaultValue(it->first, descriptions[it->first], it->second);
//		} else {
//			config->createConfigurationOption("", it->first, descriptions[it->first], OPTIONTYPE_INTEGER, it->second);
//		}
//	}
//
//	for (std::map<std::string, float>::iterator it = floatOptions.begin();
//	     it != floatOptions.end();
//	     ++it)
//	{
//		// Delete option if option exists as a different type
//		if (config->exists(it->first) && config->getOptionType("", it->first) != OPTIONTYPE_FLOAT) {
//			config->deleteConfigurationOption("", it->first);
//		}
//		// Reset default value if option exists, else create option
//		if (config->exists(it->first)) {
//			config->setDefaultValue(it->first, descriptions[it->first], it->second);
//		} else {
//			config->createConfigurationOption("", it->first, descriptions[it->first], OPTIONTYPE_FLOAT, it->second);
//		}
//	}
//}
