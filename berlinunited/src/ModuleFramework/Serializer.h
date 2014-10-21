#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <typeinfo>
#include <iostream>

#include "debug.h"

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

// serialization for common containers
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>

// hardcode serialization format
#define   SERIALIZER boost::archive::binary_oarchive
#define DESERIALIZER boost::archive::binary_iarchive
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>


/**
 * Serializer is the base class for all Serializer implementations.
 *
 * Every representation should implement this.
 */
template<class T>
class Serializer {
public:

	/** serialize function
	 **
	 ** @param representation   Representation to serialize
	 ** @param archive          Boost archive
	 **
	 ** @return true iff representation was serialized, false otherwise
	 */

	static bool serialize(
			const T& representation,
			SERIALIZER& archive)
	{
		static bool warningIssued = false;
		if (false == warningIssued) {
			WARNING("No serialization implemented for %s", typeid(T).name());
			warningIssued = true;
		}
		return false;
	}

	/** deserialize function
	 **
	 ** @param representation   Representation to write to
	 ** @param archive          Boost archive
	 **
	 ** @return true iff representation was deserialized, false otherwise
	 */

	static bool deserialize(
			T& representation,
			DESERIALIZER& archive)
	{
		static bool warningIssued = false;
		if (false == warningIssued) {
			WARNING("No deserialization implemented for %s", typeid(T).name());
			warningIssued = true;
		}
		return false;
	}
};


/*------------------------------------------------------------------------------------------------*/
//BOOST_CLASS_EXPORT(REPRESENTATION);

#define REGISTER_SERIALIZATION(REPRESENTATION, VERSION) \
	BOOST_CLASS_TRACKING(REPRESENTATION, boost::serialization::track_never)   \
	BOOST_CLASS_VERSION(REPRESENTATION, VERSION)                              \
	template<>                                                                \
	class Serializer<REPRESENTATION> {                                        \
	public:                                                                   \
		static bool serialize(                                                \
				const REPRESENTATION& representation,                         \
				SERIALIZER& archive)                                          \
		{                                                                     \
			archive & representation;                                         \
			return true;                                                      \
		}                                                                     \
		static bool deserialize(                                              \
				REPRESENTATION& representation,                               \
				DESERIALIZER& archive)                                        \
		{                                                                     \
			archive & representation;                                         \
			return true;                                                      \
		}                                                                     \
	};

#endif // SERIALIZER_H
