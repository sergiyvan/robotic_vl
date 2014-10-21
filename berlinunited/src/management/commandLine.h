/*
 * commandLine.h
 *
 *  Created on: Apr 29, 2010
 *      Author: dseifert
 */

#ifndef COMMANDLINE_H_
#define COMMANDLINE_H_

#include "utils/patterns/singleton.h"
#include "platform/system/macroMagic.h"
#include "ModuleFramework/moduleManagers.h"

#include "services.h"

#include <string>
#include <vector>
#include <map>
#include <memory>


/*------------------------------------------------------------------------------------------------*/

class CommandLine;

class CommandLineInterface {
public:
	virtual ~CommandLineInterface() {}

	virtual bool commandLineCallback(const CommandLine &cmdLine) = 0;
};


/*------------------------------------------------------------------------------------------------*/

typedef struct CommandLineCommand {
	CommandLineCommand()
		: name()
		, description()
		, moduleManagersState(nullptr)
		, callback(nullptr)
	{}

	std::string name;
	std::string description;

	ModuleManagersState *moduleManagersState;

	std::shared_ptr<CommandLineInterface> callback;
} CommandLineCommand;


/*------------------------------------------------------------------------------------------------*/

class CommandLine : public Singleton<CommandLine> {
	friend class Singleton<CommandLine>;

private:
	CommandLine();

	std::shared_ptr<CommandLineInterface> registerCommand(
			const std::string                     &commandName,
			const std::string                     &description,
			std::shared_ptr<CommandLineInterface>  callback,
			ModuleManagersState                   *moduleManagersState);

public:
	virtual ~CommandLine();

	template <typename T>
	static std::shared_ptr<T> registerCommand(
			const std::string     &commandName,
			const std::string     &description,
			ModuleManagersState   *moduleManagersState)
	{
		std::shared_ptr<T> callback = std::make_shared<T>();
		CommandLine::getInstance().registerCommand(commandName, description, std::dynamic_pointer_cast<CommandLineInterface>(callback), moduleManagersState);
		return callback;
	}

	bool process(int argc, char** argv);

	std::string getProgramName() const;
	int getCommandCount() const;

	std::string getCommand(int index) const;
	std::string getCommandOption(std::string optionName) const ;
	std::map<std::string, std::string> getCommandOptions() const;

	const CommandLineCommand &getActiveCommand() const;

	bool getCommandInfo(CommandLineCommand &cmdInfo) const;

	bool isSwitchEnabled(const std::string &switchName) const;

	void applyTo(Config& config);

private:
	int commandCount;
	std::string programName;
	std::map<std::string, std::string>        globalOptionValues;  // global options (and switches) found
	std::map<std::string, std::string>        commandOptionValues; // command options found
	std::map<std::string, CommandLineCommand> commands;            // commands found
	std::vector<std::string>                  switches;            // switches found

	void printUsageOnError(const char* wrongArgument, const char* message);
	void printUsage();
	void printDescription(const std::map<std::string, std::string> values);
};

#endif /* COMMANDLINE_H_ */
