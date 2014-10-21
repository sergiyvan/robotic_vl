#ifndef CONFIGOPTIONUNIT_H_
#define CONFIGOPTIONUNIT_H_

#include "utils/units.h"

template<typename T>
class ConfigOptionTraits<T, typename std::enable_if<is_unit<T>::value>::type>
{
public:
	static std::string getTypeName() {
		static T dummy;
		return unit2str(dummy);
	}

	static void toString(std::ostream &output, const T& value) {
		output << value.value();
	}

	static T fromString(const std::string &valueAsString) {
		return T::from_value(std::stod(valueAsString));
	}

	static bool verifyProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption) {
		return    pbOption->type() == de::fumanoids::message::ConfigurationOption_ValueType_FLOAT
		       || pbOption->type() == de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE
		       || pbOption->type() == de::fumanoids::message::ConfigurationOption_ValueType_INTEGER; // for now, for backward compatibility
	}

	static bool fromProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption, T& value) {
		if (pbOption->used() && verifyProtobuf(pbOption)) {
			if (pbOption->value().has_value_double())
				value = T::from_value(pbOption->value().value_double());
			else if (pbOption->value().has_value_float())
				value = T::from_value(pbOption->value().value_float());
			else if (pbOption->value().has_value_int())
				value = T::from_value(pbOption->value().value_int());

			return true;
		} else
			return false;
	}

	static void toProtobuf(de::fumanoids::message::ConfigurationOption* pbOption, const T& value) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE);
		pbOption->mutable_value()->set_value_double(value.value());
	}

	static void toProtobufDefault(de::fumanoids::message::ConfigurationOption* pbOption, const T& defaultValue) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE);
		pbOption->mutable_defaultvalue()->set_value_double(defaultValue.value());
	}
};


#endif /* CONFIGOPTIONUNIT_H_ */
