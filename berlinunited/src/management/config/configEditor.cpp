#include "management/commandLine.h"
#include "management/config/configEditor.h"
#include "management/config/config.h"
#include "services.h"
#include "debugging/debugging.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <cstring>
#include <vector>



/*------------------------------------------------------------------------------------------------*/

class ConfigEditorCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		std::string cmd = cmdLine.getCommand(0);

		if (cmd == "configure") {
			sleep(1);

			ConfigEditor editor( services.getConfig() );

			editor.start();
			return true;
		} else
			return false;
	}
};

namespace {
	auto cmdConfigure = CommandLine::registerCommand<ConfigEditorCmdLineCallback>("configure", "Edit the configuration", ModuleManagers::none());
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

ConfigEditor::ConfigEditor(Config &_config)
	: config(_config)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

ConfigEditor::~ConfigEditor() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 * read a string from std.
 * @param label print this before reading the input.
 * @param minLength minimal length of expected string.
 * @param maxLength maximal length of expected string.
 * @param alignment
 * @return the read string.
 */

static std::string readString(const char* label, unsigned int minLength=0, unsigned int maxLength=500, int alignment=22) {
	char formatString[100] = { 0 };
	sprintf(formatString, "%%%ds: ", alignment);
	printf(formatString, label);
	fflush(0);

	char valueString[500];
	do {
		std::cin.getline(valueString, 500);

		if (strlen(valueString) < minLength) {
			printf("Input too short. Try again:\n");
			delay(500*milliseconds);
		} else if (strlen(valueString) > maxLength)
			printf("Input too long. Try again:\n");

	} while (strlen(valueString) < minLength || strlen(valueString) > maxLength);

	return valueString;
}


/*------------------------------------------------------------------------------------------------*/

std::string ConfigEditor::queryUserSelection(const std::string &currentSection) const {
//	std::string selection = readString("Section to open", 1, 256);
//
//	if (OPTIONTYPE_INVALID == config->getOptionType(currentSection, selection)) {
//		if (selection.size() == 1) {
//			int32_t  sectionSelection = 'A' - selection[0];
//			uint32_t optionSelection = atoi(selection.c_str());
//
//			if (sectionSelection >= 0) {
//				std::vector<std::string> sections = config->getSubsections(currentSection);
//				if ((uint32_t)sectionSelection < sections.size())
//					return sections[sectionSelection];
//			}
//
//			if (optionSelection > 1) {
//				std::vector<std::string> options = config->getOptions(currentSection);
//				if (optionSelection <= options.size())
//					return options[optionSelection - 1];
//			}
//		}
//	} else {
//		return selection;
//	}

	return "";
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Start the ConfigEditor which allows editing of configurations via command line.
 **
 */

void ConfigEditor::start() {
	::Debugging::getInstance().setConsoleLogLevel(CONSOLE_LOG_WARNING);

	std::string currentSection = "";

	printf("\n\n\033[35m");
	printf("------------------------------------------------------------\n");
	printf("------- Welcome to the glorious Configuration Editor -------\n");
	printf("------------------------------------------------------------\n");
	printf("\033[0m\n");

	printUsage();

	while (true) {
		printf("\n");
		if (currentSection == "")
			printf("---- Level: ROOT -----\n");
		else
			printf("---- Level: %s -----\n", currentSection.c_str());
		printConfiguration(currentSection);
		printf("\n");
		printUsageShort();
		std::string key = readString("Action/Section", 1, 256);

		if (key.length() == 1) {
			// print configuration
			if (key == "p")
				printConfiguration(currentSection);

			// edit configuration
			else if (key == "e") {
				std::string optionToEdit = readString("Option to edit", 1, 256);
				auto option = config.getOption(Config::getFullOptionName(currentSection, optionToEdit));
				if (option != nullptr) {
					std::string value = readString("New value", 0, 255);
					option->fromString(value);
				}
			}

			// delete option/section
			else if (key == "d") {
				std::string deleteName = readString("Name of option/section to delete", 1, 256);
				auto option = config.getOption(Config::getFullOptionName(currentSection, deleteName));
				if (option != nullptr) {
					option->unset();
				}
			}

			// open sections
			else if (key == "o") {
				std::string sectionToOpen = readString("Enter name of section", 1, 256);
				if (sectionToOpen != "" && isValidSubSection(currentSection, sectionToOpen))
					currentSection = getSubsectionPath(currentSection, sectionToOpen);
			}

			//  u - go one level up
			else if (key == "u") {
				currentSection = parentSection(currentSection);
			}

			// s - save configuration
			else if (key == "s") {
				if (config.save())
					printf("Successfully saved.\n");
				else
					printf("Error saving configuration.\n");
			}

			// ? - show this help
			else if (key == "?")
				printUsage();

			// q - quit
			else if (key == "q") {
				return;
			}
		}

		// open subsection if key.length > 1
		else {
			if (isValidSubSection(currentSection, key))
				currentSection = getSubsectionPath(currentSection, key);
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 * Check if potentialSubSection is a subsection of the current section
 * @param currentSection the section we are in.
 * @param potentialSubSection the section that is a potential sub section and gets checked
 * @return true iff potentialSubSection is a sub section.
 */

bool ConfigEditor::isValidSubSection(std::string currentSection, std::string potentialSubSection) const {
	std::vector<std::string> subSections = config.getSubsections(currentSection);
	for (uint32_t i = 0; i < subSections.size(); i++) {
		if (strcasecmp(subSections[i].c_str(), potentialSubSection.c_str()) == 0)
			return true;
	}
	printf("\nERROR: Section %s is no valid subsection of %s. Try again.\n",
			potentialSubSection.c_str(), currentSection.c_str());
	return false;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Prints the available options
 **
 */

void ConfigEditor::printUsage() const {
	printf("\n");
	printf("  p - print configuration\n");
	printf("\n");
	printf("  o - open section\n");
	printf("  u - go one level up\n");
	printf("\n");
	printf("  e - edit an option\n");
	printf("  d - delete an option or section\n");
	printf("\n");
	printf("  s - save configuration\n");
	printf("\n");
	printf("  q - quit\n");
	printf("  ? - show this help\n");
	printf("\n");
	printf("  Enter the name of the section to edit it (alternativly to 'o')\n");
	printf("\n");
	printf("  Adding options/sections should be done in code. Deleting something will 'restore' it\n");
	printf("  at next start using default values.\n");
}


/*------------------------------------------------------------------------------------------------*/


/**
 * Print short usage info of the ConfigEditor
 */
void ConfigEditor::printUsageShort() const {
	printf("Actions: ");
	printf("(p)rint | ");
	printf("(o)pen section | ");
	printf("(u)p level | ");
	printf("(e)dit | ");
	printf("(d)elete | ");
	printf("(s)ave | ");
	printf("(q)uit | ");
	printf("(?) help\n");
	printf("\n");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Prints configuration
 **
 */

void ConfigEditor::printConfiguration(const std::string &section) const {
	std::vector<std::string> sections = config.getSubsections(section);
	std::vector<std::string> options  = config.getOptions(section);

	int maxOptionNameLength = 0;
	for (uint32_t i=0; i < options.size(); i++)
		maxOptionNameLength = std::max(maxOptionNameLength, int(options[i].size()));

	// output subsections
	for (uint32_t i=0; i < sections.size(); i++) {
		printf(" %c%40s\n", 'A' + i, sections[i].c_str());
	}

	// output options in this section
	for (uint32_t i=0; i < options.size(); i++) {
		printf("%02d%40s: ", i, options[i].c_str());

		std::string fullOptionName = Config::getFullOptionName(section, options[i]);
		auto option = config.getOption(fullOptionName);

		if (option->isSet() == false) {
			printf("(");
		}

		std::string value = option->toString();
		printf("%s", value.c_str());

		if (option->isSet() == false) {
			printf(")");
		}

		printf("\t\t[%s]\n", option->getTypeName().c_str());
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Determines the parent section
 **
 ** @param section  Name of section to determine parent of
 ** @return parent section of 'section' (empty string if the section or its parent is the root)
 */

std::string ConfigEditor::parentSection(const std::string &section) const {
	size_t index = section.find_last_of(".");
	if (index == section.npos)
		return "";
	else
		return section.substr(0, index);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Determines full path of sub section
 **
 ** @param section  Name of section to determine parent of
 ** @return parent section of 'section' (empty string if the section or its parent is the root)
 */

std::string ConfigEditor::getSubsectionPath(const std::string &parentSection, const std::string &subsection) const {
	if (parentSection == "")
		return subsection;
	else
		return parentSection + "." + subsection;
}
