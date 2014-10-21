#include "debug.h"
#include "signals.h"
#include "services.h"

#include "platform/system/thread.h"

#include "utils/ansiTools.h"
#include "utils/keyboard.h"

#include <execinfo.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#include <iostream>
#include <cstdlib>
#include <stdexcept>



/*------------------------------------------------------------------------------------------------*/

// backtrace data for the last exception
static const size_t maxExceptionStackDepth = 100;
static char **exceptionStackStrings;
static int exceptionStackStringDepth;


/*------------------------------------------------------------------------------------------------*/

/**
 ** Signal handler, abort execution.
 **
 ** @param sig  Signal
 */

static void abortExecution(int sig) {
	// re-enable interrupt signal (ctrl+c)
	signal(SIGINT, reinterpret_cast<sighandler_t>(0));

	ERROR("ABORTING - Press Ctrl+C again for immediate termination.");

	// release keyboard
	releaseKeyboard();

	// quit application
	class SignalHandlerTerminator : public Thread {
	public:
		virtual const char* getName() const override { return "SignalHandlerTerminator"; }
		virtual void threadMain() override {
			services.triggerTermination();
		}
	};

	SignalHandlerTerminator *terminator = new SignalHandlerTerminator();
	terminator->run();
}


/*------------------------------------------------------------------------------------------------*/

/** Print the stack trace stored in the parameters.
 **
 */

void printStackTrace(const char* const* stackStrings, int stackDepth) {
//	printf("Backtrace (%d frames):\n", stackDepth);
//	for (int i = 0; i < stackDepth; i++) {
//		printf("%s\n", stackStrings[i]);
//	}

	// output title and number of frames (minus one, because the first is this function)
	printf("Backtrace (%d frames):\n", stackDepth-1);

	// go through the backtrace, split the lines and output the info
	std::string lastAddress;

	for (int i = 1; i < stackDepth; i++) {
		// Format to parse: APPNAME(FUNCTION+OFFSET) [ADDRESS]
		const char *str = stackStrings[i];
		const char *p1 = strchr(str, '(');
		const char *p2 = strchr(p1, '+');
		const char *p3 = strchr(p1, ')');
		const char *p4 = strchr(p3, '[');
		const char *p5 = strchr(p4, ']');

		std::string execName, mangledName, offset = "???", address = "???", function = "???";

		execName = std::string(str,  int(p1 - str));
		if (p1 && p2 > p1+1)
			mangledName = std::string(p1+1, int(p2 - p1) - 1);
		if (p2 && p3 > p2+1)
			offset = std::string(p2+1, int(p3 - p2) - 1);
		if (p4 && p5 > p4+1)
			address = std::string(p4+1, int(p5 - p4) - 1);

		if (false == mangledName.empty()) {
			int status;
			char *demangledName = abi::__cxa_demangle(mangledName.c_str(), NULL, NULL, &status);
			if (demangledName) {
				function = demangledName;
				free(demangledName);
			} else {
				function = mangledName + "()";
			}

			printf("    % 3d    %s in %s function " TERM_PURPLE "%s" TERM_RESET " at offset %s\n",
					  (int)i
					, address.c_str()
					, execName.c_str()
					, function.c_str()
					, offset.c_str()
					);
		} else {
			printf("    % 3d    %s in %s\n",
					  (int)i
					, address.c_str()
					, execName.c_str()
					);
		}

		// if we get stuck in a loop, truncate the output
		if (i > 10 && lastAddress == address) {
			printf("    ... truncated\n");
			break;
		}

		lastAddress = address;
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Print the active stack trace.
 **
 */

void printStackTrace() {
	const size_t maxStackDepth = 100;
	void *stackAddresses[maxStackDepth];

	int  stackDepth = backtrace(stackAddresses, maxStackDepth);
	printStackTrace(backtrace_symbols(stackAddresses, stackDepth), stackDepth);
}



/*------------------------------------------------------------------------------------------------*/

/** Signal handler for crashes.
 **
 */

void criticalErrorHandler(int signalNumber) {
	fprintf(stderr, TERM_RED "I crashed, WHAT DID YOU DO??? I received signal %d (%s)." TERM_RESET "\n", signalNumber, strsignal(signalNumber));
	printStackTrace();
	exit(EXIT_FAILURE);
}


/*------------------------------------------------------------------------------------------------*/

/** C++ termination handler, called by unhandled exceptions.
 **
 */

void myTerminate() {
	fprintf(stderr, TERM_RED "Unhandled EXCEPTION caught." TERM_RESET "\n");
	printStackTrace(exceptionStackStrings, exceptionStackStringDepth);

	try {
		std::rethrow_exception( std::current_exception() );
	} catch (const std::exception &e) {
		if (strlen(e.what()) > 0) {
			fprintf(stderr, TERM_RED "Unhandled EXCEPTION message: %s" TERM_RESET "\n", e.what());
		}
	} catch (...) {
		auto eptr = std::current_exception();
//		fprintf(stderr, TERM_RED "Unhandled EXCEPTION is unknown." TERM_RESET "\n");
	}
	abort();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void setupSignals() {
	// intercept ctrl+c
	{
		struct sigaction signalActionCtrlC;
		signalActionCtrlC.sa_handler = abortExecution;
		signalActionCtrlC.sa_flags   = 0;
		if (   (sigemptyset(&signalActionCtrlC.sa_mask) == -1)
			|| (sigaction(SIGINT, &signalActionCtrlC, NULL) == -1))
		{
			printf("Failed to set SIGINT handler to intercept Ctrl-C.");
		}
	}

	// intercept segmentation faults
	{
		struct sigaction signalActionSegfault;
		signalActionSegfault.sa_handler = criticalErrorHandler;
		signalActionSegfault.sa_flags   = 0;
		if (   (sigemptyset(&signalActionSegfault.sa_mask) == -1)
			|| (sigaction(SIGSEGV, &signalActionSegfault, NULL) == -1))
		{
			printf("Failed to set SIGSEGV handler.");
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

// set our terminate function as part of the static initializations (before main())
static const bool SET_TERMINATE = std::set_terminate(myTerminate);



/*------------------------------------------------------------------------------------------------*/

/* GCC specific hack.
**
** Re-implement __cxa_throw (called when an exception is thrown) in order for us
** to save the stack data.
*/

#ifdef __GNUC__
extern "C" {
	void __cxa_throw(void *ex, void *info, void (*dest)(void *)) {
		// We could handle the output here, but we do not want to do this for
		// caught exceptions. So we save the data here, and use it later when
		// we catch the exception globally.
		void *stackAddresses[maxExceptionStackDepth];
		exceptionStackStringDepth = backtrace(stackAddresses, maxExceptionStackDepth);
		exceptionStackStrings = backtrace_symbols(stackAddresses, exceptionStackStringDepth);

		//
		//		fprintf(stderr, TERM_RED "EXCEPTION intercepted." TERM_RESET "\n");
		//		printStackTrace();

		// get previous instance of __cxa_throw
		static void (* const rethrow)(void*, void*, void (*)(void*)) __attribute__ ((noreturn))
			= (void (*)(void*, void*, void(*)(void*)))dlsym(RTLD_NEXT, "__cxa_throw");
		rethrow(ex, info, dest);
	}
}
#endif // __GNUC__
