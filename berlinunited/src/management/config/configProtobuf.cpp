#include "management/config/configProtobuf.h"
#include "management/config/config.h"

#include "communication/comm.h"
#include "platform/system/timer.h"
#include "platform/system/events.h"
#include "debug.h"
#include "services.h"
#include "utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <algorithm>


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */

ConfigProtobuf::ConfigProtobuf()
	: configuration()
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
 */

ConfigProtobuf::~ConfigProtobuf() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Comparison classes
 **
 */


struct OptionComparer {
	bool operator () (
	    de::fumanoids::message::ConfigurationOption const& a,
	    de::fumanoids::message::ConfigurationOption const& b) const
	{
		return strcasecmp(a.key().c_str(), b.key().c_str()) < 0;
	}
};

struct SectionComparer {
	bool operator () (
	    const de::fumanoids::message::Configuration& a,
	    const de::fumanoids::message::Configuration& b) const
	{
		return strcasecmp(a.name().c_str(), b.name().c_str()) < 0;
	}
};

// are the strings equal (case insensitive)?
static bool equalStrings(const std::string &stringA, const std::string &stringB) {
	return strcasecmp(stringA.c_str(), stringB.c_str()) == 0;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Sort
 **
 ** @param currentSection  section to sort (subsections and options)
 */

void ConfigProtobuf::sort(de::fumanoids::message::Configuration *currentSection) {
	if (NULL == currentSection)
		return;

	std::sort(currentSection->mutable_options()->begin(),  currentSection->mutable_options()->end(),  OptionComparer());
	std::sort(currentSection->mutable_sections()->begin(), currentSection->mutable_sections()->end(), SectionComparer());

	for (int i=0; i < currentSection->sections_size(); i++) {
		sort(currentSection->mutable_sections(i));
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Retrieves internal section object
 **
 ** @param sectionName   Name of section
 **
 ** @return internal object representing the section, NULL on error
 */

const de::fumanoids::message::Configuration* ConfigProtobuf::getPBSection(const std::string &sectionName) const {
	// if we are at the top, return the top-level configuration object
	if (sectionName == "") {
		return &configuration;
	}

	std::string parentSectionName = getParentSection(sectionName);
	std::string subsectionName    = getOptionName(sectionName);

	// if we are not at the top, get the parent section object (yes, this is recursive)
	// and extract the section we are looking for
	const de::fumanoids::message::Configuration* parentSection = getPBSection(parentSectionName);
	if (parentSection == 0) {
		return nullptr;
	}

	for (int i=0; i < parentSection->sections_size(); i++) {
		if (equalStrings(subsectionName, parentSection->sections(i).name())) {
			return &parentSection->sections(i);
		}
	}

	return nullptr;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Retrieves internal section object
 **
 ** @param sectionName   Name of section
 **
 ** @return internal object representing the section, NULL on error
 */

de::fumanoids::message::Configuration* ConfigProtobuf::getPBSection(const std::string &sectionName) {
	// if we are at the top, return the top-level configuration object
	if (sectionName == "") {
		return &configuration;
	}

	std::string parentSectionName = getParentSection(sectionName);
	std::string subsectionName    = getOptionName(sectionName);

	// if we are not at the top, get the parent section object (yes, this is recursive)
	// and extract the section we are looking for
	de::fumanoids::message::Configuration* parentSection = getPBSection(parentSectionName);
	if (parentSection == 0) {
		return NULL;
	}

	for (int i=0; i < parentSection->sections_size(); i++) {
		if (equalStrings(subsectionName, parentSection->sections(i).name())) {
			return parentSection->mutable_sections(i);
		}
	}

	return NULL;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Retrieves internal option object
 **
 ** @param fullOptionName   Name of option (fully qualified)
 ** @return internal object representing the option, NULL on error
 */

const de::fumanoids::message::ConfigurationOption *ConfigProtobuf::getPBOption(const std::string &fullOptionName) const {
	std::string parentSectionName = getParentSection(fullOptionName);
	std::string optionName        = getOptionName(fullOptionName);

	// get the parent section object and extract the option we are looking for
	const de::fumanoids::message::Configuration* section = getPBSection(parentSectionName);
	if (section == 0)
		return nullptr;

	for (int i=0; i < section->options_size(); i++) {
		if (equalStrings(optionName, section->options(i).key())) {
			return &section->options(i);
		}
	}

	return nullptr;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Retrieves internal option object
 **
 ** @param fullOptionName   Name of option (fully qualified)
 ** @return internal object representing the option, NULL on error
 */

de::fumanoids::message::ConfigurationOption *ConfigProtobuf::getPBOption(const std::string &fullOptionName) {
	std::string parentSectionName = getParentSection(fullOptionName);
	std::string optionName        = getOptionName(fullOptionName);

	// get the parent section object and extract the option we are looking for
	de::fumanoids::message::Configuration* section = getPBSection(parentSectionName);
	if (section == nullptr) {
		return nullptr;
	}

	for (int i=0; i < section->options_size(); i++) {
		if (equalStrings(optionName, section->options(i).key())) {
			return section->mutable_options(i);
		}
	}

	return nullptr;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Reads configuration file
 **
 ** @param  in   Input stream
 **
 ** @return true if saving was successful
**/

bool ConfigProtobuf::loadFromStream(std::istream &in) {
	bool success = configuration.ParseFromIstream(&in);

	if (success) {
		sort(&configuration);

		// for each option, go through protobuf config and extract the value
		for (auto it : options) {
			auto *pbOption = getPBOption(it.first);
			if (pbOption == nullptr)
				continue;

			it.second->fromProtobuf(pbOption);
		}

		// TODO: what to do about default values?
		services.getEvents().trigger(EVT_CONFIGURATION_LOADED, this);
	} else {
		ERROR("Error parsing configuration file. Configuration not loaded.");
	}
	return success;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void ConfigProtobuf::assemblePBConfiguration() {
	for (auto it : options) {
		de::fumanoids::message::ConfigurationOption *pbOption = getPBOption(it.first);
		if (pbOption == nullptr) {
			pbOption = createEmptyConfigurationOption(it.first);
		}

		it.second->toProtobuf(pbOption);
	}

	assert(configuration.IsInitialized());
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

const de::fumanoids::message::Configuration & ConfigProtobuf::getConfiguration() {
	assemblePBConfiguration();
	return configuration;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Saves configuration file
 **
 ** @param  filename  Name of file to save configuration in
 ** @return true if saving was successful
**/

bool ConfigProtobuf::save(const std::string &filename) {
	std::string textBackupFileName       = filename + ".text";           // human-readable version of configuration
	std::string backupTextBackupFileName = textBackupFileName + ".bak";  // backup copy of human-readable version of configuration

	// make a backup copy of the text backup
	if (fileExists(textBackupFileName.c_str())) {
		std::ifstream f1 (textBackupFileName, std::fstream::binary);
		std::ofstream f2 (backupTextBackupFileName, std::fstream::trunc | std::fstream::binary);
		f2 << f1.rdbuf ();
		f2.close();
		sync();
	}

	// save the configuration
	bool success = ConfigStorage::save(filename);

	// create text backup
	if (success) {
		saveTextBackup(textBackupFileName);
		sync();
	}

	return success;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Saves configuration file
 **
 ** @param  out   Output stream
 ** @return true if saving was successful
**/

bool ConfigProtobuf::saveToStream(std::ostream &out) {
	services.getEvents().trigger(EVT_BEFORE_CONFIG_SAVE, 0);

	assemblePBConfiguration();

	bool success = configuration.SerializeToOstream(&out);
	if (false == success) {
		ERROR("Could not serialize protobuf configuration.");
	}
	return success;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Saves a backup of the configuration in textual form.
 **
 **/

void ConfigProtobuf::saveTextBackup(const std::string filename) const {
	std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
	if (file.fail())
		return;

	file << "This is a textual backup of the protobuf configuration file." << std::endl;
	file << "Changes in this file will have no effect." << std::endl << std::endl;
	saveTextSection(configuration, 0, file);

	sync();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 **/

void ConfigProtobuf::saveTextSection(const de::fumanoids::message::Configuration& section, int indentation, std::ostream &out) const {
	for (int i=0; i < indentation; i++)
		out << "   ";
	out << section.name() << std::endl;

	indentation++;
	for (int i=0; i < section.sections_size(); i++) {
		saveTextSection(section.sections(i), indentation, out);
	}

	for (int i=0; i < section.options_size(); i++) {
		for (int j=0; j < indentation; j++)
			out << "   ";
		out << section.options(i).key() << " = ";
		switch (section.options(i).type()) {
		case de::fumanoids::message::ConfigurationOption_ValueType_STRING:
			out << section.options(i).value().value_str(); break;
		case de::fumanoids::message::ConfigurationOption_ValueType_INTEGER:
			out << section.options(i).value().value_int(); break;
		case de::fumanoids::message::ConfigurationOption_ValueType_BOOLEAN:
			out << section.options(i).value().value_bool(); break;
		case de::fumanoids::message::ConfigurationOption_ValueType_FLOAT:
			out << section.options(i).value().value_float(); break;
		case de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE:
			out << section.options(i).value().value_double(); break;
		}
		if (section.options(i).used() == false) {
			out << " (not set, using default)";
		}
		out << std::endl;
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Create a new configuration section in an existing section
 **
 ** @param parentSection  Parent section's name
 ** @param sectionName    Name of section to create (no path)
 **
 ** @return true on success
 */

bool ConfigProtobuf::createConfigurationSection(std::string parentSection, std::string sectionName) {
	if (sectionName.find_first_of('.') != sectionName.npos) {
		return false;
	}

	de::fumanoids::message::Configuration *section = getPBSection(parentSection);
	if (section == 0) {
		return false;
	}

	if (getPBSection(getFullOptionName(parentSection, sectionName)) != 0) {
		return false;
	}

	de::fumanoids::message::Configuration *newSection = section->add_sections();
	assert(newSection);
	newSection->set_name(sectionName);
	newSection->set_timestamp(getCurrentTime().value());
	newSection->set_valid(true);
	section->set_timestamp(getCurrentTime().value());
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Create a new configuration section
 **
 ** @param sectionName         full name of section
 **
 ** @return true on success
 */

bool ConfigProtobuf::createConfigurationSection(std::string sectionName) {
	std::string parentSectionName = getParentSection(sectionName);
	std::string subsectionName = getOptionName(sectionName);

	if (getPBSection(parentSectionName) == 0) {
		createConfigurationSection(parentSectionName);
	}

	return createConfigurationSection(parentSectionName, subsectionName);
}


/*------------------------------------------------------------------------------------------------*/

/** Create and initialize a new configuration option
 **
 ** @param _sectionName        section the option should be in
 ** @param _optionName         name of option
 ** @param optionDescription   description of option
 ** @param optionType          type of option
 **
 ** @return pointer to (internal) option object
 */

de::fumanoids::message::ConfigurationOption* ConfigProtobuf::createEmptyConfigurationOption(const std::string &fullName) {
	std::string parentSectionName = getParentSection(fullName);
	std::string optionName = getOptionName(fullName);

	de::fumanoids::message::Configuration *section = getPBSection(parentSectionName);
	if (section == 0) {
		createConfigurationSection(parentSectionName);
		section = getPBSection(parentSectionName);
		if (section == 0) {
			return nullptr;
		}
	}

	for (int i = 0; i < section->sections_size(); i++) {
		if (equalStrings(optionName, section->sections(i).name())) {
			return nullptr;
		}
	}

	for (int i = 0; i < section->options_size(); i++) {
		if (equalStrings(optionName, section->options(i).key())) {
			return nullptr;
		}
	}

	de::fumanoids::message::ConfigurationOption* newOption = section->add_options();
	newOption->set_key(optionName);
	newOption->set_used(false);
	newOption->mutable_value()->set_value_int(0); // value is required, so save something in it
	return newOption;
}
