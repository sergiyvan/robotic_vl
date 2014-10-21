/** @file
 **
 ** Declaration of configuration class
 **
 */

#ifndef CONFIGTEXTFILE_H
#define CONFIGTEXTFILE_H


#include <string>
#include <iostream>
#include <map>

#include "configStorage.h"


/**
 ** Configuration values (either strings or integers) can be stored in a text file.
 **
 ** Note: This class can't use the comm object to send warnings, as comm only starts
 **       after having read the configuration.
 **
 ** @ingroup config
 */

class ConfigTextFile : public ConfigStorage {
public:
	ConfigTextFile();
	virtual ~ConfigTextFile();

	virtual bool loadFromStream(std::istream &in) override;
	virtual bool saveToStream(std::ostream &out) override;
};

#endif
