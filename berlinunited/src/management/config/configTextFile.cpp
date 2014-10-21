#include "services.h"
#include "debug.h"

#include "management/config/configTextFile.h"
#include "platform/system/events.h"

#include "utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <algorithm>


/*------------------------------------------------------------------------------------------------*/

/**
 ** Removes whitespaces at beginning and end of line.
 **
 ** @param  str      String to trim
 ** @param  delims   Chars to consider as whitespace
 **
 ** @return trimmed string
 */

std::string trim(std::string const& str, char const* delims = " \t\r\n") {
	std::string result(str);
	std::string::size_type index = result.find_last_not_of(delims);
	if (index != std::string::npos)
		result.erase(++index);

	index = result.find_first_not_of(delims);
	if (index != std::string::npos)
		result.erase(0, index);
	else
		result.erase();

	return result;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */

ConfigTextFile::ConfigTextFile()
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
 */

ConfigTextFile::~ConfigTextFile() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Reads configuration file
 **
 ** @param  in   Input stream
 ** @return true if saving was successful
 */

bool ConfigTextFile::loadFromStream(std::istream &in) {
	std::string line;
	std::string key;
	std::string value;

	int lineNumber = 0;
	while (std::getline(in, line)) {
		lineNumber++;

		if (line.length() == 0)
			continue;

		// first type of comments
		if (line[0] == '#')
			continue;

		// second type of comments
		if (line[0] == ';')
			continue;

		// third type of comments
		if (line[0] == '/')
			continue;

		int posEqual = line.find('=');
		if (posEqual == 0) {
			ERROR("%d: invalid format, line ignored", lineNumber);
			continue;
		}

		// extract key
		key = trim(line.substr(0, posEqual));
		if (key.length() == 0) {
			ERROR("%d: configuration value name not found", lineNumber);
			continue;
		}
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		// extract value
		value = trim(line.substr(posEqual + 1), " \t\r\n\"");
		if (value.length() == 0) {
			ERROR("%d: configuration value not found, use \"\" for empty string!", lineNumber);
			continue;
		}

		// pass value to option if it exists
		if (exists(key)) {
			getOption(key)->fromString(value);
		}
	}

	services.getEvents().trigger(EVT_CONFIGURATION_LOADED, this);
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Saves configuration file
 **
 ** @param  out   Output stream
 ** @return true if saving was successful
 */

bool ConfigTextFile::saveToStream(std::ostream &out) {
	services.getEvents().trigger(EVT_BEFORE_CONFIG_SAVE, 0);

	for (auto it : options) {
		if (it.second->isSet())
			out << it.first << " = \"" << *it.second << "\"\n";
	}

	return true;
}
