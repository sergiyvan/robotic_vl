#include "configStorage.h"

#include "utils/utils.h"

#include <fstream>
#include <algorithm>

/*------------------------------------------------------------------------------------------------*/

/** Constructor
 **
 */

ConfigStorage::ConfigStorage()
	: configurationFilename()
{
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor
 **
 */

ConfigStorage::~ConfigStorage() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Reads configuration file
 **
 ** @param  fileName  Name of file to open
 */

bool ConfigStorage::load(std::string filename) {
	// remember file name
	configurationFilename = filename;

	// fall back to backup if configuration file is missing or empty
	if (false == fileExists(filename) || 0 == filesize(filename)) {
		std::string backupFileName = filename + ".bak";
		if (fileExists(backupFileName)) {
			WARNING("Configuration file not found or empty, trying backup copy.");
			filename = backupFileName;
		} else {
			// no config file and no backup config file
			return false;
		}
	}

	std::ifstream file(filename, std::ios::in | std::ios::binary);
	if (false == file.fail()) {
		return loadFromStream(file);
	} else {
		ERROR("Could not open configuration file '%s'", filename.c_str());
		return false;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Saves configuration file
 **
 ** @param  fileName  Name of file to save to
 */

bool ConfigStorage::save(const std::string &filename) {
	std::string backupFileName  = filename + ".bak";  // backup copy of configuration
	if (fileExists(filename.c_str())) {
		rename(filename.c_str(), backupFileName.c_str());
		sync();
	}

	std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
	if (false == file.fail()) {
		bool success = saveToStream(file);
		sync();
		return success;
	} else {
		ERROR("Could not open configuration file '%s' for saving", filename.c_str());
		return false;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Saves configuration file
 **
 */

bool ConfigStorage::save() {
	if ("" == configurationFilename) {
		ERROR("No configuration file name set.");
		return false;
	}

	return save(configurationFilename);
}
