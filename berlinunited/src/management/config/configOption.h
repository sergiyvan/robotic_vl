#ifndef CONFIG_OPTION_H_
#define CONFIG_OPTION_H_

#include "debug.h"

#include <msg_configuration.pb.h>

#include <atomic>
#include <type_traits>
#include <boost/algorithm/string/replace.hpp>


/*------------------------------------------------------------------------------------------------*/

/** Configuration options are stored in templated instances of the
 ** ConfigOptionInterface.
 **
 ** As some interface functions will not be altered in the derived classes,
 ** we implement them in the base class (even though it's not a pure interface
 ** anymore).
 **
 ** @ingroup config
 */

class ConfigOptionInterface {
protected:

	/// describes which value is currently active (see ConfigOption<T> for use)
	enum class ActiveValueType : uint8_t {
		  DEFAULTVALUE  = 0
		, STORAGEVALUE  = 1
		, OVERRIDEVALUE = 2
	};

	/** Constructor, protected as the interface will never be constructed pure.
	 **
	 ** @param name          Name of the configuration option
	 ** @param description   Description of the configuration option
	 */
	ConfigOptionInterface(const std::string name, const std::string &description)
		: activeValueType(ActiveValueType::DEFAULTVALUE)
		, optionName(name)
		, optionDescription(description)
		, hasStoredValue(false)
		, lockCount(0)
	{
		cs.setName("ConfigOption " + optionName);
	}

public:

	/// Destructor
	virtual ~ConfigOptionInterface() {}

	/** Set an override value that should be used until the next set operation,
	 ** but which should not be saved.
	 **
	 ** @param value   String representation of the value to set
	 */
	virtual void setOverride(const std::string &value) {
		fromString(value, ActiveValueType::OVERRIDEVALUE);
	}

	/** Get the name of this configuration option
	 **
	 ** @return name of configuration option
	 */
	const std::string &getName() const {
		return optionName;
	}

	/** Get the description of this configuration option
	 **
	 ** @return description of configuration option
	 */
	const std::string &getDescription() const {
		return optionDescription;
	}

	/** Get the printable name of the type.
	 **
	 ** @return name of the type stored in this configuration option
	 */
	virtual std::string getTypeName() const = 0;

	/** Checks whether a value was set. Override and default values do not matter,
	 ** and this function does not take into account which of the values is active.
	 **
	 ** @return true iff there is a user-defined (non-override) value set
	 */
	bool isSet() {
		return hasStoredValue;
	}

	/** Unsets the option.
	 */
	virtual void unset() = 0;

	/** Checks whether this option is locked.
	 **
	 ** @return true iff option is locked
	 */
	bool isLocked() {
		return (0 != lockCount.load());
	}

	/** Lock this option (changes are not applied until it is unlocked)
	 */
	void lock() {
		CriticalSectionLock lock(cs);

		lockCount++;
	}

	/** Unlock this option.
	 */
	virtual void unlock() = 0;

	/** Streams the contents (active value) into an output stream
	 **
	 ** @param output   Output stream
	 ** @param o        configuration option to stream into o.
	 ** @return reference to the output stream
	 */
	friend std::ostream &operator<<(std::ostream &output, const ConfigOptionInterface &o) {
		o.toString(output);
		return output;
	}

	/** Convert the stored or (if no stored value) default value to a string
	 **
	 ** return the active value as string
	 */
	virtual std::string toString() const = 0;

	/** Convert the stored or (if no stored value) default value to a string
	 **
	 ** @param output
	 */
	virtual void toString(std::ostream &output) const = 0;

	/** Set the stored value from the string.
	 **
	 ** @param valueAsString  String representation of the value to set
	 */
	virtual void fromString(const std::string &valueAsString) = 0;

	/** Set a selectable value from the string.
	 **
	 ** @param valueAsString  String representation of the value to set
	 ** @param valueType      Which value to set (stored, default, override)
	 */
	virtual void fromString(const std::string &valueAsString, ActiveValueType valueType) = 0;

	/** Serialize the configuration option to protobuf.
	 **
	 ** @param pbOption    Protobuf message to serialize to
	 */
	virtual void toProtobuf(de::fumanoids::message::ConfigurationOption* pbOption) const = 0;

	/** Deserialize a protobuf message into this configuration option.
	 **
	 ** @param pbOption    Protobuf message to serialize from
	 */
	virtual void fromProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption) = 0;

protected:

	/// describes which value is currently active
	ActiveValueType activeValueType;

	/// the name of the configuration option
	std::string optionName;

	/// the description of the configuration option
	std::string optionDescription;

	/// whether a stored value is set
	bool hasStoredValue;

	/// the lock count (0 if not locked)
	std::atomic<uint32_t> lockCount;

	CriticalSection cs;
};



/*------------------------------------------------------------------------------------------------*/

template<typename T, typename=void>
class ConfigOptionTraits {
};

#include "configOptionTraits/configOptionFloating.hpp"
#include "configOptionTraits/configOptionIntegral.hpp"
#include "configOptionTraits/configOptionString.hpp"
#include "configOptionTraits/configOptionUnit.hpp"
#include "configOptionTraits/configOptionDuration.hpp"


/*------------------------------------------------------------------------------------------------*/

/** ConfigOption<T> stores a configuration option of type T.
 **
 ** @ingroup config
 */

template <typename T>
class ConfigOption
	: public ConfigOptionInterface
{
	typedef ConfigOptionTraits<T> Traits;

public:
	/** Constructor
	 **
	 ** @param name          Name of the configuration option
	 ** @param defaultValue  Default value, to be used if no explicit value is set
	 ** @param description   Description of the configuration option
	 */
	ConfigOption(const std::string &name, const T& defaultValue, const std::string &description)
		: ConfigOptionInterface(name, description)
		, activeValue(defaultValue)
		, currentlyStoredValue(defaultValue)
		, defaultValue(defaultValue)
	{
		activeValueType = ActiveValueType::DEFAULTVALUE;
	}

	/** Destructor
	 */
	virtual ~ConfigOption() {
	}

	/** Get the printable name of the type.
	 **
	 ** @return
	 */
	virtual std::string getTypeName() const override {
		return Traits::getTypeName();
	}

	/** Unset the stored value. An override value will remain active.
	 */
	virtual void unset() {
		CriticalSectionLock lock(cs);

		hasStoredValue = false;
		if (activeValueType == ActiveValueType::STORAGEVALUE) {
			activeValueType = ActiveValueType::DEFAULTVALUE;
			activeValue = defaultValue;
		}
	}

	/** Set the stored value.
	 **
	 ** @param newValue  Value to set
	 */
	void set(const T& newValue) {
		set(newValue, ActiveValueType::STORAGEVALUE);
	}

	/** Set a value
	 **
	 ** @param newValue   Value to set
	 ** @param valueType  Type of value to set (default, stored, override)
	 */
	void set(const T& newValue, ActiveValueType valueType) {
		CriticalSectionLock lock(cs);

		// if option is locked, queue the value away
		if (isLocked()) {
			switch (valueType) {
				case ActiveValueType::STORAGEVALUE: {
					// setting a storage value will remove an override value
					auto it = queuedValues.find(ActiveValueType::OVERRIDEVALUE);
					if (it != queuedValues.end()) {
						queuedValues.erase(it);
					}

					queuedValues[ActiveValueType::STORAGEVALUE] = newValue;
					break;
				}

				case ActiveValueType::OVERRIDEVALUE:
					if (queuedValues.find(ActiveValueType::STORAGEVALUE) != queuedValues.end()) {
						WARNING("Queueing override for config option %s, which already has a storage value queued", optionName.c_str());
					}

					queuedValues[ActiveValueType::OVERRIDEVALUE] = newValue;
					break;

				case ActiveValueType::DEFAULTVALUE:
					queuedValues[ActiveValueType::DEFAULTVALUE] = newValue;
					break;

				default: assert(false); break;
			}

		// otherwise (i.e. option is not locked), apply the values
		} else {
			switch (valueType) {
				case ActiveValueType::STORAGEVALUE:
					activeValueType = ActiveValueType::STORAGEVALUE;
					activeValue = currentlyStoredValue = newValue;
					hasStoredValue = true;
					break;

				case ActiveValueType::OVERRIDEVALUE:
					activeValueType = ActiveValueType::OVERRIDEVALUE;
					activeValue = newValue;
					break;

				case ActiveValueType::DEFAULTVALUE:
					defaultValue = newValue;
					break;

				default: assert(false); break;
			}
		}
	}

	/** Retrieves the active value.
	 **
	 ** @return the active value
	 */
	const T& get() const {
		return activeValue;
	}

	/** Retrieve the active value, using a custom default value.
	 **
	 ** @param defaultValue   Default value to use
	 ** @return the stored/override value or, if neither is set, the supplied default value if
	 */
	const T& get(const T& defaultValue) const {
		if (activeValueType == ActiveValueType::DEFAULTVALUE) {
			return defaultValue;
		} else {
			return activeValue;
		}
	}

	/** Unlock this option. Apply any queued values.
	 */
	virtual void unlock() {
		CriticalSectionLock lock(cs);

		if (false == isLocked()) {
			ERROR("Unlock mismatch for configuration option %s", optionName.c_str());
			return;
		}

		if (--lockCount == 0) {
			// if we have queued values, apply them
			if (false == queuedValues.empty()) {
				if (queuedValues.count(ActiveValueType::DEFAULTVALUE))
					set(queuedValues[ActiveValueType::DEFAULTVALUE], ActiveValueType::DEFAULTVALUE);
				if (queuedValues.count(ActiveValueType::STORAGEVALUE))
					set(queuedValues[ActiveValueType::STORAGEVALUE], ActiveValueType::STORAGEVALUE);
				if (queuedValues.count(ActiveValueType::OVERRIDEVALUE))
					set(queuedValues[ActiveValueType::OVERRIDEVALUE], ActiveValueType::OVERRIDEVALUE);

				queuedValues.clear();
			}
		}
	}

	virtual std::string toString() const {
		std::stringstream ss;
		Traits::toString(ss, activeValue);
		return ss.str();
	}

	virtual void toString(std::ostream &output) const {
		Traits::toString(output, activeValue);
	}

	/** Set the configuration option from a value encoded as a string.
	 **
	 ** @param valueAsString   String to extract new value from
	 */
	virtual void fromString(const std::string &valueAsString) {
		fromString(valueAsString, ActiveValueType::STORAGEVALUE);
	}

	/** Set the configuration option from a value encoded as a string.
	 **
	 ** @param valueAsString   String to extract new value from
	 ** @param valueType       Type of value to set (storage, default, override)
	 */
	virtual void fromString(const std::string &valueAsString, ActiveValueType valueType) {
		T newValue;

		try {
			newValue = Traits::fromString(valueAsString);
		} catch ( ... ) {
			printf("Invalid value '%s' for option %s (type %s)\n", valueAsString.c_str(), optionName.c_str(), Traits::getTypeName().c_str());
			return;
		}

		set(newValue, valueType);
	}

	/** Set the configuration option from the value stored in the protobuf option.
	 **
	 ** @param pbOption   Pointer to the protobuf configuration option.
	 */
	virtual void fromProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption) {
		assert(nullptr != pbOption);

		if (false == Traits::verifyProtobuf(pbOption)) {
			ERROR("Option %s mismatch in registered type (%s) vs protobuf type (%s)", optionName.c_str(), Traits::getTypeName().c_str(), getPBTypeName(pbOption->type()));
			return;
		}

		T value;

		// if option is not used, switch back to default value
		if (false == pbOption->used()) {
			// set the default value as the active value
			set(defaultValue);

			// remove the flag that we have a stored value
			hasStoredValue = false;

		// if option is set, try to extract it and then set it
		} else if (true == Traits::fromProtobuf(pbOption, value)) {
			set(value);
		}
	}

	/** Convert the configuration option to a protobuf configuration option
	 **
	 ** @param pbOption   Pointer to the protobuf configuration option
	 */
	virtual void toProtobuf(de::fumanoids::message::ConfigurationOption* pbOption) const {
		assert(nullptr != pbOption);

		CriticalSectionLock lock(cs);

		// set the meta data
		pbOption->set_used(hasStoredValue);
		pbOption->set_description(optionDescription);
		pbOption->set_unit(getTypeName());

		// set the values
		Traits::toProtobufDefault(pbOption, defaultValue);
		if (hasStoredValue) {
			Traits::toProtobuf(pbOption, activeValue);
		} else { // at least one value must be set
			pbOption->mutable_value()->set_value_int(0);
		}
	}


private:

	/** little helper to get a string representation of a Protobuf configuration type.
	 **
	 ** @param type   Protobuf configuration type
	 ** @return string representation
	 */
	const char* getPBTypeName(::de::fumanoids::message::ConfigurationOption_ValueType type) {
		switch (type) {
		case ::de::fumanoids::message::ConfigurationOption_ValueType_STRING:  return "string";
		case ::de::fumanoids::message::ConfigurationOption_ValueType_INTEGER: return "int";
		case ::de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE:  return "double";
		case ::de::fumanoids::message::ConfigurationOption_ValueType_FLOAT:   return "float";
		default: return "unknown";
		}
	}


protected:
	/// the currently active value
	T activeValue;

	/// the value that is or will be stored in the configuration file
	T currentlyStoredValue;

	/// the default value
	T defaultValue;

	/// a copy of the value(s) to use when unlocked
	std::map<ActiveValueType, T> queuedValues;
};

#endif
