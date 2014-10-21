#include "logPlayer.h"

#include "debugging/logging/logFileHeader.h"
#include "debugging/logging/logFrameHeader.h"
#include "debugging/logging/logRepresentationHeader.h"

#include "ModuleFramework/ModuleManager.h"

#include "management/config/config.h"

#include "utils/ansiTools.h"
#include "utils/keyboard.h"
#include "utils/stringTools.h"
#include "utils/utils.h"


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgPlayLog  = ConfigRegistry::registerOption<std::string>("play", "", "The log files to use");
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

bool LogPlayer::isActive() {
	if ("" == cfgPlayLog->get())
		return false;

	std::vector<std::string> tokens;
	split(cfgPlayLog->get(), ":", tokens);
	if (fileExists(tokens[0]))
		return true;

	static bool errorIssued = false;
	if (!errorIssued) {
		ERROR("Log file %s not found. Starting in live mode.", cfgPlayLog->get().c_str());
		errorIssued = true;
	}
	return false;
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

bool LogPlayer::isActive(const std::string &moduleManagerName) {
	return header.moduleManagerName == moduleManagerName;
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

LogPlayer::LogPlayer()
	: ifs(nullptr)
	, archive(nullptr)
{
	std::string cfg = cfgPlayLog->get();
	if (cfg.find(";") != cfg.npos) {
		throw std::runtime_error("Only one module manager can be played back (at the moment)");
	}

	std::vector<std::string> tokens;
	split(cfg, ":", tokens);

	// extract list of explicit specified modules to run
	modules.clear();
	if (tokens.size() >= 2)
		split(tokens[1], ",", modules);

	logFileName = tokens[0];

	ifs = new std::ifstream(logFileName, std::ios::in | std::ios::binary);
	archive = new DESERIALIZER(*ifs);

	archive->register_type<LogFileHeader>();
	archive->register_type<LogFrameHeader>();
	archive->register_type<LogRepresentationHeader>();

	*archive & header;

	// also add the modules that were logged to be enabled
	modules.insert( modules.end(), header.moduleNames.begin(), header.moduleNames.end() );
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

LogPlayer::~LogPlayer() {
}


/*------------------------------------------------------------------------------------------------*/

/*
**
*/

void LogPlayer::start(ModuleManager* moduleManager) {
	this->moduleManager = moduleManager;
	Thread::run();
}



/*------------------------------------------------------------------------------------------------*/

/*
**
*/

void LogPlayer::threadMain() {
	// activate all requested modules
	for (const auto& module : modules) {
		moduleManager->setModuleEnabled(module, true, false);
	}

	// calculate the execution list based on the representation dependencies
	moduleManager->calculateExecutionList();

	// start the manager
	moduleManager->ModuleManager::startManager(1);

	// disable all modules that were not explicitly enabled
	for (const auto& module : moduleManager->getExecutionList()) {
		if (std::find(modules.begin(), modules.end(), module) == modules.end())
			moduleManager->setModuleEnabled(module, false, false);
	}

	representations = header.representationNames;

	takeKeyboard();

	printf(TERM_BLUE
		"====================================================================\n"
		"LogPlayer Mode\n"
		"   Log file %s\n"
		"       -> build info: %s\n"
		"       -> recorded from %s\n"
		"   x frames serialized\n"
		"====================================================================\n" TERM_RESET
		"\n"
		"usage:\n"
		" - SPACE:    process next frame\n"
		"\n"
		, logFileName.c_str()
		, header.buildInfo.c_str()
		, header.moduleManagerName.c_str()
		);

	printf("Modules executed:\n");
	moduleManager->printExecutionList();

	// loop for keyboard control
	while (isRunning()) {
		int c = getKeyWithUsTimeout(500);
		if (c == ' ') {
			deserialize(moduleManager->getBlackBoard().getRegistry());
			moduleManager->executeModules();
		}
	}
	
	releaseKeyboard();
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

void LogPlayer::deserialize(const BlackBoard::Registry &registry) {
	LogFrameHeader header;
	*archive & header;

	INFO("Deserializing frame %d", header.framenumber);

	auto dataPos = ifs->tellg();

	for (uint32_t i = 0; i < header.repCount; i++) {
		LogRepresentationHeader header;
		*archive & header;

		auto it = registry.find(representations[header.id]);
		if (registry.end() == it) {
			ERROR("Representation %s not found", representations[header.id].c_str());
		} else {
			it->second->getRepresentation().deserialize(*archive);
		}
	}

	if ( (uint32_t)(ifs->tellg() - dataPos) != header.framesize ) {
		ERROR("Mismatch in de-serialization. Expected to read %d bytes, but read %d bytes.",
				header.framesize,
				(uint32_t)(ifs->tellg() - dataPos));

		// correct position
		ifs->seekg((int)dataPos + header.framesize);
	}
}
