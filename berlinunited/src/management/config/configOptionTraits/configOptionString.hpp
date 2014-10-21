#ifndef CONFIGOPTIONSTRING_HPP_
#define CONFIGOPTIONSTRING_HPP_


template<>
class ConfigOptionTraits<std::string>
{
	typedef std::string T;

public:
	static std::string getTypeName() {
		return "string";
	}

	static void toString(std::ostream &output, const T& value) {
		output << value;
	}

	static T fromString(const std::string &valueAsString) {
		return valueAsString; // sheer elegance in its simplicity
	}

	static bool verifyProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption) {
		return (pbOption->type() == de::fumanoids::message::ConfigurationOption_ValueType_STRING);
	}

	static bool fromProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption, T& value) {
		if (pbOption->used() && verifyProtobuf(pbOption)) {
			if (pbOption->value().has_value_str()) {
				value = pbOption->value().value_str();
				return true;
			}
		}

		return false;
	}

	static void toProtobuf(de::fumanoids::message::ConfigurationOption* pbOption, const T& value) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_STRING);
		pbOption->mutable_value()->set_value_str(value);
	}

	static void toProtobufDefault(de::fumanoids::message::ConfigurationOption* pbOption, const T& defaultValue) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_STRING);
		pbOption->mutable_defaultvalue()->set_value_str(defaultValue);
	}
};


#endif /* CONFIGOPTIONSTRING_HPP_ */
