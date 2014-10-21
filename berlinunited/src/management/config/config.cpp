#include "config.h"
#include "configProtobuf.h"

#include "communication/comm.h"
#include "management/commandLine.h"

#include "services.h"

#include <msg_configuration.pb.h>


/*------------------------------------------------------------------------------------------------*/

/** Constructor.
 */


Config::Config() {
	cs.setName("Config");
}


/*------------------------------------------------------------------------------------------------*/

/** Assignment operator.
 */

Config& Config::operator=(const Config &other) {
	options  = other.options;
	sections = other.sections;

	return *this;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

Config::~Config() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param optionName
 */

void Config::registerOptionInSections(const std::string &optionName, std::shared_ptr<ConfigOptionInterface> option) {
	std::string sectionName = getParentSection(optionName);
	if (sectionName != "") {
		getSection(sectionName)->addChild(option);
		registerOptionInSections(sectionName, option);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

std::vector<std::string> Config::getSubsections(const std::string &section) const {
	std::vector<std::string> result;

	for (auto it : sections) {
		if (section == getParentSection(it.first))
			result.push_back(getOptionName(it.first));
	}

	return result;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

std::vector<std::string> Config::getOptions(const std::string &section) const{
	std::vector<std::string> result;

	for (auto it : options) {
		if (section == getParentSection(it.first))
			result.push_back(getOptionName(it.first));
	}

	return result;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void Config::printOptions(bool includingValues, bool onlyChangedOptions) {
	// collect all option names
	std::vector<std::string> optionNames;
	int maximumLength = 0;
	for (auto it : options) {
		const std::string &name = it.second->getName();
		optionNames.push_back(name);
		maximumLength = std::max(maximumLength, (int)name.length());
	}

//	optionNames.sort

	// output options in one alphabetical list
	for (auto optionName : optionNames) {
		const int MaxValueLength = 15;

		std::string optionType = options[optionName]->getTypeName();
		std::stringstream value;

		if (options[optionName]->isSet()) {
			value << *options[optionName];
		} else {
			value << "(" << *options[optionName] << ")";
		}

		if (!onlyChangedOptions || options[optionName]->isSet()) {
			std::cout << std::left << "  ";
			std::cout << std::setw(maximumLength) << optionName;
			std::cout << std::setw(0) << " - " << std::setw(10) << optionType;
			if (includingValues)
				std::cout << std::setw(MaxValueLength) << value.str();
			std::cout << std::setw(0) << " " << options[optionName]->getDescription();
			std::cout << std::endl;
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Determines the fully qualified option name.
 **
 ** @param sectionName   Name of section
 ** @param optionName    Name of option
 **
 ** @return fully qualified option name
 */

std::string Config::getFullOptionName(const std::string &sectionName, const std::string &optionName) {
	std::string fullname;

	if (sectionName == "")
		fullname = optionName;
	else if (sectionName[sectionName.size() - 1] == '.')
		fullname = sectionName + optionName;
	else
		fullname = sectionName + "." + optionName;

	std::transform(fullname.begin(), fullname.end(), fullname.begin(), ::tolower);
	return fullname;
}


/*------------------------------------------------------------------------------------------------*/

/** Given a fully qualified option name, return the full section path
 **
 ** @param fullOptionName  Fully qualified option name
 **
 ** @return section the option resides in, or empty string if the parent section is the root
 */

std::string Config::getParentSection(const std::string &fullOptionName) {
	// empty option names have an empty parent
	if (fullOptionName == "")
		return "";

	// if the option name ended with "." (section separator), discard it and
	// rather get the parent section of that section
	if (fullOptionName[fullOptionName.size() - 1] == '.')
		return getParentSection(fullOptionName.substr(0, fullOptionName.length() - 1));

	// get the parent section
	size_t p = fullOptionName.find_last_of('.');
	if (p != fullOptionName.npos)
		return fullOptionName.substr(0, p);
	else
		return "";
}


/*------------------------------------------------------------------------------------------------*/

/** Given a fully qualified option name, return the option name without path components
 **
 ** @param fullOptionName   Fully qualified option name
 **
 ** @return name of the option
 */

std::string Config::getOptionName(const std::string &fullOptionName) {
	size_t p = fullOptionName.find_last_of('.');
	if (p != fullOptionName.npos)
		return fullOptionName.substr(p + 1);
	else
		return fullOptionName;

}


/*------------------------------------------------------------------------------------------------*/

/** Check whether an option of the given name exists.
 **
 ** @param name   Name of the option to check existence for
 **
 ** @return true if the option exists
 */

bool Config::exists(std::string name) const {
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	return (options.end() != options.find(name));
}


/*------------------------------------------------------------------------------------------------*/

/** Return the section of the given name. If it does not exist yet, it is created
 **
 ** @param name   Name of the section to retrieve
 **
 ** @return a shared pointer to the section
 */

std::shared_ptr<ConfigSection> Config::getSection(std::string name) {
	// make option name lowercase
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	if (sections.count(name) > 0) {
		return sections[name];
	} else {
		sections[name] = std::make_shared<ConfigSection>(name);
		return sections[name];
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Get a sorted vector of all configuration option names.
 **
 ** @return alphabetically sorted vector of all options.
 */

std::vector<std::string> Config::getAllOptionNames() {
	std::vector<std::string> allOptions;

	for (const auto &it : options)
		allOptions.push_back(it.first);

	return allOptions;
}


/*------------------------------------------------------------------------------------------------*/

/** Take the configuration values stored in the protobuf message and apply them
 ** to this Config object
 **
 ** @param rootSectionName   Name of the root section that pbConfig represents
 ** @param pbConfig          Protobuf configuration
 */

void Config::fromPBConfig(const std::string rootSectionName, const ::de::fumanoids::message::Configuration& pbConfig) {
	for (int i=0; i < pbConfig.options_size(); i++) {
		std::shared_ptr<ConfigOptionInterface> option = getOption(getFullOptionName(rootSectionName, pbConfig.options(i).key()));
		option->fromProtobuf(&pbConfig.options(i));
	}

	for (int i=0; i < pbConfig.sections_size(); i++) {
		fromPBConfig(getFullOptionName(rootSectionName, pbConfig.sections(i).name()), pbConfig.sections(i));
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Message callback: save/load config etc.
 **
 ** @param messageName    Name/type of message
 ** @param msg            Received message
 ** @param senderID       ID of sender
 **
 ** @return true if message was processed
 */

bool Config::messageCallback(const std::string& messageName,
                             const google::protobuf::Message& msg,
                             int32_t id,
                             RemoteConnectionPtr &remote)
{
	using namespace de::fumanoids;

	message::Message response;
	const message::ConfigurationRequest& configRequestMsg = (const message::ConfigurationRequest&)msg;
	message::ConfigurationRequest* responseRequest = response.MutableExtension(message::configurationRequest);

	// should we inform FUremote
	bool shouldSend = false;

	// if our configuration is to be set, copy the new config
	if (   configRequestMsg.has_setconfiguration()
	    && configRequestMsg.setconfiguration()
	    && configRequestMsg.has_configuration()
	    && id == services.getID())
	{
		std::string nodeName = configRequestMsg.configuration().name();

		fromPBConfig(nodeName, configRequestMsg.configuration());
		responseRequest->set_configurationwasset(true);
		services.getEvents().trigger(EVT_CONFIGURATION_LOADED, this);
		shouldSend = true;
	}

	// if our configuration was requested, return it
	if (   configRequestMsg.has_configurationrequest()
	    && configRequestMsg.configurationrequest()
	    && id == services.getID())
	{
		// the protobuf configuration storage is compatible with the communication
		// format, so "mis-use" the existing functionality by creating a new
		// configuration instance
		ConfigProtobuf pbConfig;
		(Config&)pbConfig = *this;
		::de::fumanoids::message::Configuration pb = pbConfig.getConfiguration();

		// sort, #2373 bugfix improved
		ConfigProtobuf::sort(&pb);

		*(responseRequest->mutable_configuration()) = pb;
		shouldSend = true;
	}

	if (   configRequestMsg.has_saveconfiguration()
	    && configRequestMsg.saveconfiguration()
	    && id == services.getID())
	{
		save();
		responseRequest->set_configurationwassaved(true);
		shouldSend = true;
	}

	// if requested, send the data to the remote party
	if (shouldSend) {
		// send configuration
		services.getComm().sendMessage(response, remote.get());
	}

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Trigger this configuration object to sent an update notification event.
 **
 */

void Config::triggerUpdateNotification() {
	services.getEvents().trigger(EVT_CONFIGURATION_LOADED, this);
}


/*------------------------------------------------------------------------------------------------*/

class ConfigCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		std::string cmd = cmdLine.getCommand(0);

		if (cmd == "showconfig") {
			bool onlyChanged = true;
			if (cmdLine.getCommand(1) == "all")
				onlyChanged = false;

			std::cout << std::endl;
			std::cout << "Configuration values";
			if (onlyChanged)
				std::cout << " (only set values, run 'showconfig all' for all values)";
			else
				std::cout << " (default values in parentheses)";
			std::cout << std::endl << std::endl;

			services.getConfig().printOptions(true, onlyChanged);

			std::cout << std::endl;
			return true;
		} else
			return false;
	}
};

namespace {
	auto cmdShowConfig = CommandLine::registerCommand<ConfigCmdLineCallback>("showconfig", "Show the configuration", ModuleManagers::none());
}
