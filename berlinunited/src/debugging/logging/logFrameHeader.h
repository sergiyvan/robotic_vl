#ifndef LOGFRAMEHEADER_H_
#define LOGFRAMEHEADER_H_

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>

#include <string>
#include <inttypes.h>

#include "platform/system/timer.h"


/*------------------------------------------------------------------------------------------------*/

/** \class LogFrameHeader
 ** \brief Header before each log frame.
 */

class LogFrameHeader {
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive &ar, const unsigned int fileVersion) {
		ar & timestamp;
		ar & framenumber;
		ar & framesize;
		ar & repCount;
	}

public:
	robottime_t timestamp; // TODO: which time?
	uint32_t    framenumber;
	uint32_t    framesize;      //!< frame data length (in bytes) without header
	uint16_t    repCount;       //!< how many representations are serialized
};

BOOST_CLASS_VERSION(LogFrameHeader, 1)
BOOST_CLASS_TRACKING(LogFrameHeader, boost::serialization::track_never)


#endif
