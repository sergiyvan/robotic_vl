#ifndef LOGWRITER_H_
#define LOGWRITER_H_

#include "ModuleFramework/BlackBoard.h"
#include "ModuleFramework/ModuleManager.h"
#include "ModuleFramework/Serializer.h"

#include <string>
#include <fstream>

class ModuleManager;

class LogWriter {
public:
	LogWriter(ModuleManager* moduleManager);
	virtual ~LogWriter();

	static bool isConfigured(const std::string &managerName);

	void addRepresentation(const std::string &representationName);

	void start();
	void start(const std::string &directory);
	void start(const std::string &directory, const std::string &filename);
	void stop();

	void serialize(int framenumber, const BlackBoard::Registry &registry);

protected:
	void serialize(Representation& representation);

	ModuleManager *manager;

	// logging
	std::ofstream *ofs;
	SERIALIZER    *archive;

	/// whether to log all representations
	bool logAll;

	/// the list of modules that will be logged
	std::vector<std::string> moduleNames;

	/// the list of representations that will be logged
	std::vector<std::string> representations;

	/// a mapping of representation name to index into #representations
	std::map<std::string, uint16_t> rep2ID;

	/// flag whether the log file header has been written yet
	bool isHeaderWritten;
};


#endif /* LOGWRITER_H_ */
