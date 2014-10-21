#ifndef __CONFIG_H__
#define __CONFIG_H__

/*------------------------------------------------------------------------------------------------*/

/**
 ** @defgroup config Configuration
 ** @ingroup management
 **
 ** Base class for configuration providers.
 **
 ** The configuration can be stored in whatever ways available providers support. This class
 ** defines the common interface to access configuration data. See ConfigTextFile and
 ** ConfigProtobuf for some implementations.
 **
 ** The configuration is split into sections, whereas each section may have subsections, ad
 ** infinum. Each section may contain options that hold values. This way a tree structure is
 ** created.
 **
 ** As common representation, a string is used. Sections are separated by a dot ('.') and each
 ** provider should ensure that a section does not contain a value (i.e. an option with the
 ** same name) to avoid any confusion and to support providers that can not handle this kind
 ** of duplicity.
 **
 ** The empty section "" is the root of the configuration.
 **
 ** A fully qualified option name (or section name) is a string that contains the full path
 ** of an option (or section) starting at the root. For example: "vision.hardware.cameratype"
 **
 ** Naming rules:
 **  - all sections and options may only contain the following characters:
 **           a..z 0..9 - _
 **  -> only lowercase
 **  -> no dots or slashes
 **
 */


#include <string>
#include <map>
#include <vector>

#include "configOption.h"
#include "communication/messageRegistry.h"

#include <msg_configuration.pb.h>


class ConfigSection;

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

class Config
		: public MessageCallback
{
public:
	Config();
	virtual ~Config();

	// copy assignment operator
	Config& operator=(const Config &other);

	// the base class does not support loading/saving
	virtual bool load(std::string filename)        { return false; }
	virtual bool save(const std::string &filename) { return false; }
	virtual bool save()                            { return false; }

	/** Register a new configuration option
	 **
	 ** @param name          Name of the option, use a dot as name separator
	 ** @param defaultValue  Default value
	 ** @param description   A description for this option
	 **
	 ** @return a reference to the configuration option
	 */
	template <typename T>
	std::shared_ptr<ConfigOption<T>> registerOption(std::string name, const T& defaultValue, const std::string &description) {
		CriticalSectionLock lock(cs);

		// make option name lowercase
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		// create configuration option
		std::shared_ptr<ConfigOption<T>> option = std::make_shared<ConfigOption<T>>(name, defaultValue, description);
		registerOptionInSections(name, std::dynamic_pointer_cast<ConfigOptionInterface>(option));
		options[name] = option;

		// check whether a derived class (ConfigStorage etc) has a value cached
		applyPreloadedValue(name);

		return option;
	}

	// check whether the option exists
	bool exists(std::string name) const;

	/** Check whether an option of the given type exists
	 **
	 ** @param name  Name of the option to check
	 ** @return true if an option with that name and that type exists
	 */
	template <typename T>
	bool exists(std::string name) const {
		// make option name lowercase
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		auto it = options.find(name);
		if (it != options.end()) {
			if (nullptr != std::dynamic_pointer_cast<ConfigOption<T>>(it->second))
				return true;
		}

		return false;
	}

	// get a configuration section representation
	std::shared_ptr<ConfigSection> getSection(std::string name);

	/** Retrieve an option (non-const, base class)
	 **
	 ** @param name  name of the configuration option
	 ** @return a shared pointer to the base option instance
	 */
	std::shared_ptr<ConfigOptionInterface> getOption(std::string name) {
		return std::const_pointer_cast< ConfigOptionInterface >(
				static_cast<const Config&>(*this).getOption(name)
		);
	}

	/** Retrieve an option (const, base class)
	 **
	 ** @param name  name of the configuration option
	 ** @return a shared pointer to the base option instance
	 */
	std::shared_ptr<const ConfigOptionInterface> getOption(std::string name) const {
		// make option name lowercase
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		const auto &it = options.find(name);
		if (it != options.end()) {
			return it->second;
		} else {
			throw std::runtime_error("Unknown option " + name);
		}
	}

	/** Retrieve an option (non-const)
	 **
	 ** @param name  name of the configuration option
	 ** @return a shared pointer to the option instance
	 */
	template <typename T>
	std::shared_ptr<ConfigOption<T>> getOption(std::string name) {
		return std::const_pointer_cast< ConfigOption<T> >(
				static_cast<const Config&>(*this).getOption<T>(name)
		);
	}

	/** Retrieve an option (const)
	 **
	 ** @param name  name of the configuration option
	 ** @return a shared pointer to the (const) option instance
	 */
	template <typename T>
	std::shared_ptr<const ConfigOption<T>> getOption(std::string name) const {
		// make option name lowercase
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		const auto &it = options.find(name);
		if (it != options.end()) {
			std::shared_ptr<ConfigOption<T>> option = std::dynamic_pointer_cast<ConfigOption<T>>(it->second);
			if (nullptr == option) {
				std::string msg = "Accessing option " + name + " (of type " + it->second->getTypeName() + ") with wrong type";
				throw std::runtime_error(msg);
			}
			return option;
		} else {
			throw std::runtime_error("Unknown option " + name);
		}
	}

	/** Derived configuration classes (namely those that load configuration files)
	 ** may have a value for this configuration option cached that should now be
	 ** applied.
	 **
	 ** @param option   Option to apply value to
	 */
	virtual void applyPreloadedValue(std::string name) {}

	/** Set an option value.
	 **
	 ** @param name  name of the configuration option
	 ** @param value value to set
	 */
	template <typename T>
	void set(const std::string &name, const T& value) {
		getOption<T>(name)->set(value);
	}

	/** Retrieve an option value. If not set, use the default value.
	 **
	 ** @param name  name of the configuration option
	 ** @return the current value of this configuration option
	 */
	template <typename T>
	const T& get(const std::string &name) const {
		return getOption<T>(name)->get();
	}

	/** Retrieve an option value, if not set use a custom default value
	 **
	 ** @param name  name of the configuration option
	 ** @param defaultValue  a custom default value
	 ** @return the current value of this configuration option
	 */
	template <typename T>
	const T& get(const std::string &name, const T& defaultValue) const {
		return getOption<T>(name)->get(defaultValue);
	}

	// get all registered option names
	std::vector<std::string> getAllOptionNames();

	// get all section names within the given section
	std::vector<std::string> getSubsections(const std::string &section) const;

	// get all option names within the given section
	std::vector<std::string> getOptions(const std::string &section) const;

	// print the options to screen
	void printOptions(bool includingValues, bool onlyChangedOptions=false);

	// some functions to assemble/parse option names
	static std::string getFullOptionName(const std::string& sectionName, const std::string& optionName);
	static std::string getParentSection(const std::string &fullOptionName);
	static std::string getOptionName(const std::string &fullOptionName);

	// trigger an update notification event from this instance
	void triggerUpdateNotification();

protected:
	// register a new option in all the parent sections
	void registerOptionInSections(const std::string &optionName, std::shared_ptr<ConfigOptionInterface> option);

	// set the options below the given section from the given protobuf configuration message
	void fromPBConfig(const std::string rootSectionName, const ::de::fumanoids::message::Configuration& pbConfig);

	// callback handler for incoming protobuf requests
	virtual bool messageCallback(
		const std::string               &messageName,
		const google::protobuf::Message &msg,
		int32_t                          senderID,
		RemoteConnectionPtr             &remote) override;

	CriticalSection cs;

	// the actual data
	std::map< std::string, std::shared_ptr<ConfigOptionInterface> > options;
	std::map< std::string, std::shared_ptr<ConfigSection>         > sections;

};

#include "configRegistry.h"
#include "configSection.h"

#endif
