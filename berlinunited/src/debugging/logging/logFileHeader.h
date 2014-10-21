#ifndef LOGFILEHEADER_H_
#define LOGFILEHEADER_H_

#include "logFrameHeader.h"
#include "logRepresentationHeader.h"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>

#include <string>
#include <vector>

#include "platform/system/timer.h"


/*------------------------------------------------------------------------------------------------*/

/** \class LogFileHeader
 ** \brief The most relevant information about a log file, stored at the beginning.
 */

class LogFileHeader {
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive &ar, const unsigned int fileVersion) {
		if (1 == fileVersion) {
			ar & timestamp;
			ar & moduleManagerName;
			ar & buildInfo;
			ar & moduleNames;
			ar & representationNames;

			// boost stores additional information in the archive the first
			// time a class is serialized. As we are overwriting both the
			// frame and the representation header during serialization (in
			// order to update the data size), this would corrupt our archive.
			// So, as a little hack, we write a dummy frame and representation
			// header here, resolving this issue once and for all :-)
			LogFrameHeader          dummyFrame;
			LogRepresentationHeader dummyRepresentation;
			ar & dummyFrame;
			ar & dummyRepresentation;

		} else {
			assert(false);
		}
	}

public:
	LogFileHeader()
		: timestamp(0*milliseconds)
	{}

	/// date/time of log
	robottime_t timestamp;

	/// name of the module manager being logged
	std::string moduleManagerName;

	/// build info
	std::string buildInfo;

	/// The representations which are logged. The index into this vector will
	/// also serve as an ID for the representation
	std::vector<std::string> representationNames;

	/// The modules that are being logged
	std::vector<std::string> moduleNames;
};


BOOST_CLASS_VERSION(LogFileHeader, 1)
BOOST_CLASS_TRACKING(LogFileHeader, boost::serialization::track_never)

#endif
