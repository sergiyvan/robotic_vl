/*
 * commandLine.cpp
 *
 *  Created on: Apr 29, 2010
 *      Author: dseifert
 */

#include "management/commandLine.h"
#include "management/config/config.h"

#include <string.h>
#include <stdio.h>

#include <sstream>
#include <algorithm>


/*------------------------------------------------------------------------------------------------*/

/** Constructor
 **
 */

CommandLine::CommandLine()
	: commandCount(0)
	, programName()
	, globalOptionValues()
	, commandOptionValues()
	, commands()
	, switches()
{
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor
 **
 */

CommandLine::~CommandLine() {
}


/*------------------------------------------------------------------------------------------------*/

/** Print usage information on error.
 **
 ** @param wrongArgument   Argument that caused the error
 ** @param message         Error message
 */

void CommandLine::printUsageOnError(const char* wrongArgument, const char* message) {
	printf("Error: %s %s\n", wrongArgument, message);
	printf("Call %s --help for help.\n", getProgramName().c_str());
}


/*------------------------------------------------------------------------------------------------*/

/** Print general usage information
 **
 */

void CommandLine::printUsage() {
	printf("Usage:\n\n");
	printf("    %s [global options] [global switches] commands [options]\n\n", getProgramName().c_str());
	printf("where options are of the form '--key value' or '--key=value'\n");
	printf("and switches are '--switch'.\n");
	printf("\nSupported global options:\n");
	services.getConfig().printOptions(false, false);

	printf("\nSupported global switches: (prefix with '--')\n");
	ConfigRegistry::getInstance().printSwitches();
	printf("\nSupported commands:\n");

	std::map<std::string, CommandLineCommand>::const_iterator it;
	int maxLength = 15;
	for (it = commands.begin(); it != commands.end(); ++it)
		maxLength = std::max(maxLength, (int)it->first.size());

	for (it = commands.begin(); it != commands.end(); ++it)
		printf("  %s%s- %s\n", it->first.c_str(), std::string(maxLength - it->first.size() + 1, ' ').c_str(), it->second.description.c_str());

	printf("\n");
}


/*------------------------------------------------------------------------------------------------*/

/** Parses the command line arguments and stores them for later use.
 **
 ** @param argc  Number of arguments
 ** @param argv  Arguments
 **
 ** @return true iff parsing was successful
 */

bool CommandLine::process(int argc, char** argv) {
	// the command line is constructioned two-fold:
	//   1. a list of options (global options) or switches
	//   2. a command following by optional parameters or options

	bool workingOnGlobalOptions = true;

	if (argc > 0)
		programName = argv[0];

	// go through list and get all data
	for (int i=1; i<argc; i++) {
		int len = strlen(argv[i]);

		// handle special case
		if (strcmp(argv[i], "--help") == 0) {
			printUsage();
			return false;
		}

		// handle switches and options
		if (argv[i][0] == '-') {

			// values starting with "-" better start with "--" ;-)
			if (len < 3 || argv[i][1] != '-') {
				printUsageOnError(argv[i], "is not a valid option, should be --optionname");
				return false;
			}

			// options can be "--optioname" "optionvalue" or "--optionname=optionvalue"
			char* keyStr   = argv[i] + 2;
			char* valueStr = strstr(argv[i], "=");

			// separate value string for "--optionname=optionvalue" cases
			if (valueStr)
				*valueStr++ = 0;

			// keys must be lowercase
			for (char* lowerKeyStr = keyStr; *lowerKeyStr != 0; lowerKeyStr++)
				*lowerKeyStr = tolower(*lowerKeyStr);

			if (!workingOnGlobalOptions || services.getConfig().exists(keyStr)) {
				if (valueStr == 0 && (i+1 == argc || argv[i+1][0] == '-')) {
					printUsageOnError(argv[i], "is not a valid option, as it is not followed by an option value");
					return false;
				}

				if (valueStr == 0)
					valueStr = argv[++i];

				if (workingOnGlobalOptions)
					globalOptionValues[keyStr] = valueStr;
				else
					commandOptionValues[keyStr] = valueStr;

			} else if (ConfigRegistry::getInstance().isSwitch(keyStr)) {
				switches.push_back(keyStr);

			} else {
				printUsageOnError(argv[i], "is not a recognized option or switch.");
				return false;
			}
		} else {
			workingOnGlobalOptions = false;

			std::stringstream optionName;
			optionName << "command" << commandCount++;
			commandOptionValues[optionName.str()] = argv[i];
		}
	}

	return true;
}



/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @param config
 */

void CommandLine::applyTo(Config& config) {
	for (auto it : globalOptionValues) {
		config.getOption(it.first)->setOverride(it.second);
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Registers a command name for the command line.
 **
 ** @param commandName      name of command
 ** @param description      a short description of this command
 ** @param callback         callback handler
 ** @param moduleManagersState
 **                         an instance of the module managers state object,
 **                         specifying which module managers should be run
 **                         for this command and with which runlevel
 */

std::shared_ptr<CommandLineInterface> CommandLine::registerCommand(
		const std::string                     &commandName,
		const std::string                     &description,
		std::shared_ptr<CommandLineInterface>  callback,
		ModuleManagersState                   *moduleManagersState)
{
	if (nullptr != callback) {
		CommandLineCommand cmd;
		cmd.name                = commandName;
		cmd.description         = description;
		cmd.moduleManagersState = moduleManagersState;
		cmd.callback            = callback;

		commands[cmd.name]      = cmd;
	}

	return callback;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

const CommandLineCommand& CommandLine::getActiveCommand() const {
	static CommandLineCommand cmd;

	std::map<std::string, CommandLineCommand>::const_iterator it = commands.find(getCommand(0));
	if (it == commands.end())
		return cmd;
	else {
		return it->second;
	}

}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool CommandLine::getCommandInfo(CommandLineCommand &cmdInfo) const {
	std::map<std::string, CommandLineCommand>::const_iterator it = commands.find(getCommand(0));
	if (it == commands.end())
		return false;
	else {
		cmdInfo = it->second;
		return true;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

std::string CommandLine::getProgramName() const {
	return programName;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

int CommandLine::getCommandCount() const {
	return commandCount;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

std::string CommandLine::getCommand(int index) const {
	std::stringstream optionName;
	optionName << "command" << index;

	std::map<std::string, std::string>::const_iterator it = commandOptionValues.find(optionName.str());
	if (it == commandOptionValues.end())
		return "";
	else
		return it->second;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

std::string CommandLine::getCommandOption(std::string key) const {
	std::map<std::string, std::string>::const_iterator it = commandOptionValues.find(key);
	if (it == commandOptionValues.end())
		return "";
	else
		return it->second;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

std::map<std::string, std::string> CommandLine::getCommandOptions() const {
	return commandOptionValues;
}


/*------------------------------------------------------------------------------------------------*/

/** Given the name of a switch, say whether it is enabled.
 **
 ** @param switchName    name of switch
 **
 ** @return true if switch is enabled (e.g. specified on command line)
 */

bool CommandLine::isSwitchEnabled(const std::string &switchName) const {
	return (std::find(switches.begin(), switches.end(), switchName) != switches.end());

}
