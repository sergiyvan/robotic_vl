#ifndef CONFIGOPTIONFLOATING_H_
#define CONFIGOPTIONFLOATING_H_

template<typename T>
class ConfigOptionTraits<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
{
public:
	static std::string getTypeName() {
		int status;
		std::unique_ptr<char[], void (*)(void*)> result(
				abi::__cxa_demangle(typeid(T).name(), 0, 0, &status), std::free);
		std::string name = result.get() ? std::string(result.get()) : "???";
		boost::replace_all(name, "unsigned ", "u");
		return name;
	}

	static void toString(std::ostream &output, const T& value) {
		output << value;
	}

	static T fromString(const std::string &valueAsString) {
		return std::stod(valueAsString);
	}

	static bool verifyProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption) {
		return    pbOption->type() == de::fumanoids::message::ConfigurationOption_ValueType_FLOAT
		       || pbOption->type() == de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE;
	}

	static bool fromProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption, T& value) {
		if (pbOption->used() && verifyProtobuf(pbOption)) {
			if (pbOption->value().has_value_double()) {
				value = pbOption->value().value_double();
			} else {
				value = pbOption->value().value_float();
			}
			return true;
		}

		return false;
	}

	static void toProtobuf(de::fumanoids::message::ConfigurationOption* pbOption, const T& value) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE);
		pbOption->mutable_value()->set_value_double(value);
	}

	static void toProtobufDefault(de::fumanoids::message::ConfigurationOption* pbOption, const T& defaultValue) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_DOUBLE);
		pbOption->mutable_defaultvalue()->set_value_double(defaultValue);
	}
};



#endif /* CONFIGOPTIONFLOATING_H_ */
