/*
 * configOptionDuration.hpp
 *
 *  Created on: Aug 14, 2014
 *      Author: tobias
 */

#ifndef CONFIGOPTIONDURATION_HPP_
#define CONFIGOPTIONDURATION_HPP_

#include <chrono>

namespace detail {


	template<typename>
	struct is_duration : std::false_type {};

	template<typename Rep, typename Period>
	struct is_duration<std::chrono::duration<Rep, Period>> : std::true_type {};


	template<typename T>
	std::string getTypeName() {
		int status;
		std::unique_ptr<char[], void (*)(void*)> result(
				abi::__cxa_demangle(typeid(T).name(), 0, 0, &status), std::free);
		std::string name = result.get() ? std::string(result.get()) : "???";
		boost::replace_all(name, "unsigned ", "u");
		return name;
	}

	template<>
	inline std::string getTypeName<std::chrono::nanoseconds>() {
		return "std::chrono::nanoseconds";
	}

	template<>
	inline std::string getTypeName<std::chrono::microseconds>() {
		return "std::chrono::microseconds";
	}

	template<>
	inline std::string getTypeName<std::chrono::milliseconds>() {
		return "std::chrono::milliseconds";
	}

	template<>
	inline std::string getTypeName<std::chrono::seconds>() {
		return "std::chrono::seconds";
	}

	template<>
	inline std::string getTypeName<std::chrono::minutes>() {
		return "std::chrono::minutes";
	}

	template<>
	inline std::string getTypeName<std::chrono::hours>() {
		return "std::chrono::hours";
	}

}


template<typename T>
class ConfigOptionTraits<T, typename std::enable_if<detail::is_duration<T>::value and std::is_integral<typename T::rep>::value>::type>
{
public:
	static std::string getTypeName() {
		return detail::getTypeName<T>();
	}

	static void toString(std::ostream &output, const T& value) {
		output << value.count();
	}

	static T fromString(const std::string &valueAsString) {
		return static_cast<T>(std::stol(valueAsString));
	}

	static bool verifyProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption) {
		return (pbOption->type() == de::fumanoids::message::ConfigurationOption_ValueType_INTEGER);
	}

	static bool fromProtobuf(const de::fumanoids::message::ConfigurationOption* pbOption, T& value) {
		if (pbOption->used() && verifyProtobuf(pbOption)) {
			if (pbOption->value().has_value_int()) {
				value = static_cast<T>(pbOption->value().value_int());
				return true;
			}
		}

		return false;
	}

	static void toProtobuf(de::fumanoids::message::ConfigurationOption* pbOption, const T& value) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_INTEGER);
		pbOption->mutable_value()->set_value_int(value.count());
	}

	static void toProtobufDefault(de::fumanoids::message::ConfigurationOption* pbOption, const T& defaultValue) {
		assert(nullptr != pbOption);

		pbOption->set_type(de::fumanoids::message::ConfigurationOption_ValueType_INTEGER);
		pbOption->mutable_defaultvalue()->set_value_int(defaultValue.count());
	}

};



#endif /* CONFIGOPTIONDURATION_HPP_ */
