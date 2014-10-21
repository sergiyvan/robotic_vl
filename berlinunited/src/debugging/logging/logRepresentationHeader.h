#ifndef LOGREPRESENTATIONHEADER_H_
#define LOGREPRESENTATIONHEADER_H_

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include <string>
#include <inttypes.h>

class LogRepresentationHeader {
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive &ar, const unsigned int fileVersion) {
		ar & id;
		ar & size;
	}

public:
	uint16_t    id;        //!< id of serialized representation
	uint32_t    size;      //!< representation data length (in bytes) without header
};

BOOST_CLASS_VERSION(LogRepresentationHeader, 1)
BOOST_CLASS_TRACKING(LogRepresentationHeader, boost::serialization::track_never)


#endif /* LOGREPRESENTATIONHEADER_H_ */
