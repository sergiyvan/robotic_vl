#include "management/commandLine.h"

#include <gtest/gtest.h>


/*------------------------------------------------------------------------------------------------*/
//
class GTestCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		if (cmdLine.getCommandCount() != 1) {
			printf("Incorrect command count\n");
			return false;
		}

		// fake command line arguments
		std::map<std::string, std::string> options = cmdLine.getCommandOptions();
		int argc = options.size() + 1;
		char** argv = new char*[argc];

		argv[0] = strdup(cmdLine.getProgramName().c_str());

		std::map<std::string, std::string>::iterator it = options.begin();
		for (int i=1; i<argc; i++) {
			std::string option = "--" + it->first + "=" + it->second;
			it++;
			argv[i] = strdup(option.c_str());
		}

		::testing::InitGoogleTest(&argc, argv);

		for (int i=0; i<argc; i++)
			free(argv[i]);
		delete[] argv;
		argv = 0;
		bool __attribute__ ((unused)) success = RUN_ALL_TESTS();
		return true;
	}
};

namespace {
	auto cmd = CommandLine::registerCommand<GTestCmdLineCallback>("unittests", "Perform unit tests (Use '--gtest_filter=TestName.Function*' for filtering)", ModuleManagers::none());
}


/*------------------------------------------------------------------------------------------------*/

class GTestXmlCmdLineCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		// fake command line arguments
		int argc = 2;
		char* argv[2];
		argv[0] = strdup(cmdLine.getProgramName().c_str());
		argv[1] = strdup("--gtest_output=xml:gtest.xml");
		::testing::InitGoogleTest(&argc, argv);
		for (int i=0; i < argc; i++)
			free(argv[i]);
		return RUN_ALL_TESTS() > 0;
	}
};

namespace {
	auto cmdXml = CommandLine::registerCommand<GTestXmlCmdLineCallback>("unittestsxml", "Perform unit tests with xml output", ModuleManagers::none());
}
