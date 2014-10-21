#ifndef LOGPLAYER_H_
#define LOGPLAYER_H_

#include "ModuleFramework/BlackBoard.h"
#include "ModuleFramework/ModuleManager.h"
#include "ModuleFramework/Serializer.h"

#include "logFileHeader.h"

#include "utils/units.h"

#include <string>
#include <fstream>

class LogPlayer
	: public Thread
{
public:
	LogPlayer();
	virtual ~LogPlayer();

	virtual const char* getName() const override {
		return "LogPlayer";
	}

	static bool isActive();
	bool isActive(const std::string &moduleManagerName);

	void start(ModuleManager* moduleManager);

	virtual void threadMain() override;

protected:
	std::string   logFileName;
	LogFileHeader header;

	uint32_t currentFrameNumber;

	ModuleManager *moduleManager;

	std::vector<std::string> modules;
	std::vector<std::string> representations;

	// logging
	std::ifstream *ifs;
	DESERIALIZER  *archive;

	void deserialize(const BlackBoard::Registry &registry);
};


#endif
