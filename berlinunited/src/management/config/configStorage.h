#ifndef __CONFIG_STORAGE_H__
#define __CONFIG_STORAGE_H__

/*------------------------------------------------------------------------------------------------*/

/**
@ingroup config

The ConfigStorage handles the loading/saving of a set of configuration values
in a file. A configuration option consists of the following elements:

 - a name
 - a description
 - a default value
 - the set value (optional)

Different storage providers may support different types of elements to be saved.

 */


#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "config.h"


class ConfigStorage : public Config {
protected:
	std::string configurationFilename;

public:
	ConfigStorage();
	virtual ~ConfigStorage();

	virtual bool load(std::string filename) override;
	virtual bool loadFromStream(std::istream &in) = 0;

	virtual bool save(const std::string &filename) override;
	virtual bool save() override;
	virtual bool saveToStream(std::ostream &out) = 0;
};

#endif
