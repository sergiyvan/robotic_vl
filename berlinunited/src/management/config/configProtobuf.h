#ifndef CONFIGPROTOBUF_H
#define CONFIGPROTOBUF_H

#include "configStorage.h"

#include <msg_configuration.pb.h>

#include <string>
#include <iostream>
#include <map>


/*------------------------------------------------------------------------------------------------*/

/**
 ** configuration provider based on protobuf.
 **
 ** @note This class can't use the comm object to send warnings, as comm only starts
 **       after having read the configuration.
 **
 ** @ingroup config
 */

class ConfigProtobuf : public ConfigStorage {
	friend class Config;

protected:
	de::fumanoids::message::Configuration configuration;

	      de::fumanoids::message::Configuration       *getPBSection(const std::string &sectionName);
	const de::fumanoids::message::Configuration       *getPBSection(const std::string &sectionName) const;
	      de::fumanoids::message::ConfigurationOption *getPBOption(const std::string &fullOptionName);
	const de::fumanoids::message::ConfigurationOption *getPBOption(const std::string &fullOptionName) const;

	bool createConfigurationSection(std::string parentSection, std::string sectionName);
	bool createConfigurationSection(std::string sectionName);
	de::fumanoids::message::ConfigurationOption* createEmptyConfigurationOption(const std::string &fullOptionName);

	void assemblePBConfiguration();

	void saveTextBackup(const std::string filename) const;
	void saveTextSection(const de::fumanoids::message::Configuration& section, int indentation, std::ostream &out) const;

	static void sort(de::fumanoids::message::Configuration *currentSection);

	const de::fumanoids::message::Configuration & getConfiguration();

public:
	ConfigProtobuf();
	virtual ~ConfigProtobuf();

	virtual bool loadFromStream(std::istream &in) override;

	virtual bool save() override {
		return ConfigStorage::save();
	}
	virtual bool save(const std::string &filename) override;
	virtual bool saveToStream(std::ostream &out) override;


	/** Derived configuration classes (namely those that load configuration files)
	 ** may have a value for this configuration option cached that should now be
	 ** applied.
	 **
	 ** @param name   Name of option to apply value to
	 */
	virtual void applyPreloadedValue(std::string name) {
		// make option name lowercase
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		de::fumanoids::message::ConfigurationOption *pbConfig = getPBOption(name);
		if (nullptr != pbConfig)
			getOption(name)->fromProtobuf(pbConfig);
	}
};

#endif
