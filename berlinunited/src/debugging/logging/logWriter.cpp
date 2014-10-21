#include "logWriter.h"
#include "services.h"

#include "debugging/logging/logFileHeader.h"
#include "debugging/logging/logFrameHeader.h"
#include "debugging/logging/logRepresentationHeader.h"

#include "ModuleFramework/ModuleManager.h"

#include "management/config/config.h"
#include "utils/utils.h"
#include "utils/stringTools.h"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

#include <boost/algorithm/string/join.hpp>


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgLog      = ConfigRegistry::registerOption<std::string>("log",     "",     "Which modules/representations to log.");
	auto cfgLogDir   = ConfigRegistry::registerOption<std::string>("logdir",  "log/", "Directory to store log files into");
}


/*------------------------------------------------------------------------------------------------*/

/** Check whether a module manager is configured for logging.
 **
 ** @param managerName Name of module manager
 ** @return true if the module manager is configured for logging
 */

bool LogWriter::isConfigured(const std::string &managerName) {
	const std::string cfg = cfgLog->get();

	if (cfg == "") {
		return false;
	} else if (cfg == "all") {
		return true;
	} else if (cfg.substr(0, managerName.size() + 1) == managerName + ":") {
		return true;
	} else if (cfg.find("," + managerName + ":") != cfg.npos) {
		return true;
	}

	return false;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** LOG FILE FORMAT
 **
 ** The format of the log file is as follows:
 **
 ** FILE HEADER
 **     uint32_t size of header
 **     string
 **
 ** FRAME (repeated)
 **     FRAME HEADER
 **         uint64_t timestamp  (in milliseconds) (TODO: which time?)
 **         uint32_t framenumber
 **         uint32_t frame data length (in bytes) without header
 **     FRAME DATA
 **         REPRESENTATION (repeated)
 **             representation name
 **             serialized representation
 **
 */


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

LogWriter::LogWriter(ModuleManager *moduleManager)
	: manager(moduleManager)
	, ofs(nullptr)
	, archive(nullptr)
	, logAll(false)
	, isHeaderWritten(false)
{
	std::string managerName = manager->getName();
	assert(LogWriter::isConfigured(managerName));

	std::string cfg = cfgLog->get();
	if ("all" == cfg) {
		logAll = true;
		INFO("Logging enabled for manager %s for all representations", managerName.c_str());
		return;
	}

	// if no colon is found, no module manager is specified, so we assume
	// the value is a filename
	if (cfg.npos == cfg.find(":")) {
		// TODO: read log information from configuration file
		ERROR("No module managers specified in %s", cfgLog->getName().c_str());
		return;
	}

	std::vector<std::string> moduleManagerTokens;
	split(cfg, ";", moduleManagerTokens);

	bool managerFound = false;
	std::string tokenString;
	for (auto &moduleManagerToken : moduleManagerTokens) {
		if (moduleManagerToken.substr(0, managerName.size() + 1) == managerName + ":") {
			managerFound = true;
			tokenString = moduleManagerToken.substr(managerName.size() + 1);
		}
	}

	if (false == managerFound) {
		// this should not really happen, as a logwriter should only be
		// instantiated if a configuration is available
		assert(false);
		return;
	}

	if ("all" == tokenString) {
		logAll = true;
		INFO("Logging enabled for manager %s for all representations", managerName.c_str());
		return;
	}

	std::vector<std::string> tokens;
	split(tokenString, ",", tokens);

	for (auto token : tokens) {
		bool isModule = false;
		char first    = token.front();
		char last     = token.back();

		if ('+' == first  || '-' == first) {
			isModule = true;
			token.erase(0, 1);
		}
		if ('+' == last || '-' == last) {
			isModule = true;
			token.pop_back();
		}

		AbstractModuleCreator* module = manager->getModule(token);
		if (false == isModule && nullptr == module) {
			addRepresentation(token);
		} else if (nullptr == module) {
			// unknown module, ignore it
		} else {
			moduleNames.push_back(token);

			if (first != '-') {
				for (const auto &rep : module->getModule()->getRequiredRepresentations())
					addRepresentation(rep->getName());
				for (const auto &rep : module->getModule()->getRecycledRepresentations())
					addRepresentation(rep->getName());
			}
			if (last != '-') {
				for (const auto &rep : module->getModule()->getProvidedRepresentations())
					addRepresentation(rep->getName());
			}
		}
	}

	INFO("Logging enabled for manager %s for the representations %s",
			managerName.c_str(),
			boost::algorithm::join(representations, ", ").c_str());
}



/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

LogWriter::~LogWriter() {
	delete archive;
	delete ofs;
}


/*------------------------------------------------------------------------------------------------*/

/* Adds a representation name to the list of representations that should be logged.
**
** @param representationName      Representation name to be added
*/

void LogWriter::addRepresentation(const std::string &representationName) {
	// add name only if representation has not yet been added
	if (std::find(representations.begin(), representations.end(), representationName) == representations.end())
		representations.push_back(representationName);
}


/*------------------------------------------------------------------------------------------------*/

/* Sets up logging and flags the recording. The log file location/name will
** be determined automatically.
*/

void LogWriter::start() {
	if (fileExists(cfgLogDir->get())) {
		start(cfgLogDir->get());
	} else {
		ERROR("LOG: Directory %s does not exist, creating log files in current directory", cfgLogDir->get().c_str());
		start(".");
	}
}


/*------------------------------------------------------------------------------------------------*/

/* Sets up logging and flags the recording. The log file will be created in the
** given directory and named automatically.
*/

void LogWriter::start(const std::string &directory) {
	assert(nullptr != manager);

	std::stringstream ss;
	time_t t = time(NULL); // current time
	struct tm* now = localtime(&t);

	// assemble filename: NAME-year_month_day_hour_min_sec-MANAGER.log
	ss << services.getName()    << "-"       // name of robot
	   << (now->tm_year + 1900)
	   << std::setfill('0')     << std::setw(2) << (now->tm_mon+1)
	   << std::setfill('0')     << std::setw(2) << (now->tm_mday) << "T"
	   << std::setfill('0')     << std::setw(2) << (now->tm_hour)
	   << std::setfill('0')     << std::setw(2) << (now->tm_min)
	   << std::setfill('0')     << std::setw(2) << (now->tm_sec)  << "-"
	   << manager->getName()    << ".log";

	start(directory, ss.str());
}


/*------------------------------------------------------------------------------------------------*/

/* Sets up logging and flags the recording.
**
*/

void LogWriter::start(const std::string &directory, const std::string &filename) {
	assert(nullptr != manager);
	assert(nullptr == ofs);
	assert(nullptr == archive);

	ofs = new std::ofstream(directory + "/" + filename, std::ios::out | std::ios::binary);
	archive = new SERIALIZER(*ofs);

	// register the management classes
	archive->register_type<LogFileHeader>();
	archive->register_type<LogFrameHeader>();
	archive->register_type<LogRepresentationHeader>();
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

void LogWriter::stop() {
	assert(nullptr != archive);
	delete archive;
	archive = nullptr;

	assert(nullptr != ofs);
	ofs->close();
	delete ofs;
	ofs = nullptr;
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

void LogWriter::serialize(int framenumber, const BlackBoard::Registry &registry) {
	assert(nullptr != archive);

	// write header
	if (false == isHeaderWritten) {
		LogFileHeader header;
		header.timestamp         = getCurrentTime();
		header.moduleManagerName = manager->getName();
		header.moduleNames       = moduleNames;

		if (logAll) {
			for (auto &it : registry)
				header.representationNames.push_back( it.second->getRepresentation().getName() );
		} else {
			header.representationNames = representations;
		}

		// assemble mapping of representation name to id
		for (uint16_t i=0; i < header.representationNames.size(); i++)
			rep2ID[header.representationNames[i]] = i;

		*archive & header;
		isHeaderWritten = true;
	}

	// remember position of header (we will overwrite the header later)
	int headerPos = ofs->tellp();

	// write initial header
	LogFrameHeader frameHeader;
	frameHeader.timestamp   = getCurrentTime();
	frameHeader.framenumber = framenumber;
	frameHeader.framesize   = 0;
	frameHeader.repCount    = 0;
	*archive & frameHeader;

	// remember current position in stream
	int dataPos = ofs->tellp();

	// serialize representations
	int representationCount = 0;
	if (logAll) {
		for (auto &it : registry) {
			serialize(it.second->getRepresentation());
			representationCount++;
		}
	} else {
		for (const auto& representationName : representations) {
			if (registry.find(representationName) == registry.end())
				throw std::runtime_error("Unknown representation " + representationName);

			serialize( registry.find(representationName)->second->getRepresentation() );
			representationCount++;
		}
	}
	int endPos = ofs->tellp();

	// update frame header
	frameHeader.framesize = endPos - dataPos;
	frameHeader.repCount = representationCount;

	ofs->seekp(headerPos);
	*archive & frameHeader;
	assert(ofs->tellp() == dataPos);
	ofs->seekp(endPos);
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

void LogWriter::serialize(Representation& representation) {
	LogRepresentationHeader header;
	header.id   = rep2ID[ representation.getName() ];
	header.size = 0;

	auto headerPos = ofs->tellp();
	*archive & header;

	auto dataPos = ofs->tellp();
	representation.serialize(*archive);

	auto endPos = ofs->tellp();

	ofs->seekp(headerPos);
	header.size = endPos - dataPos;
	*archive & header;

	assert(ofs->tellp() == dataPos);
	ofs->seekp(endPos);

//	printf("  serialized %s (header %d, data %d)\n", representation.getName().c_str(), (int)headerPos, (int)dataPos);
}
