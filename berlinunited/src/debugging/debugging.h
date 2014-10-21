#ifndef _DEBUGGING_H_
#define _DEBUGGING_H_

#include "communication/remoteConnection.h"
#include "communication/messageRegistry.h"
#include "utils/patterns/singleton.h"

#include "msg_debuggingcommands.pb.h"
#include "msg_3d.pb.h"

#include <map>
#include <string>

#include <stdarg.h>

class ImageDebugger;


/** @addtogroup debug
 * @{
 */


/**
 ** This enum holds all available option types and
 ** should correlate with its opposite in
 ** msg_debuggingcommands.proto
 **
 */
typedef enum {
	TEXT              = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_TEXT,
	PLOTTER           = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_PLOTTER,
	PLOTTER3D         = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_PLOTTER3D,
	TABLE             = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_TABLE,
	STOPWATCH         = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_STOPWATCH,
	DRAWING_CAMERA    = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_DRAWING_CAMERA,
	DRAWING_FIELD     = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_DRAWING_FIELD,
	DRAWING_RELATIVE  = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_DRAWING_RELATIVE,
	DRAWING_VGA       = de::fumanoids::message::DebuggingCommands_DebuggingOptionType_DRAWING_VGA,
} DebuggingOptionType;


/* REGISTER_DEBUG option flags */
#define BASIC   0      /* no extra */
#define ENABLED (1<<0) /* enabled by default */
#define CMDLOUT (1<<1) /* print to cmdline */
#define NONET   (1<<2) /* do not send debug output via any network, applies only to TEXT */


/**
 ** DebuggingOption describes a single option with
 ** its name, type and current status.
 **
 */
typedef struct DebuggingOption {
	DebuggingOption()
		: name()
		, description()
		, srcLocation()
		, type()
		, enabled(false)
		, cmdlineout(false)
		, netout(false)
	{}

	std::string name;
	std::string description;
	std::string srcLocation;

	DebuggingOptionType type;

	bool enabled;
	bool cmdlineout;
	bool netout;
} DebuggingOption;

typedef enum {
	  CONSOLE_LOG_PERMIT_ALL   = 0
	, CONSOLE_LOG_INFO         = 1
	, CONSOLE_LOG_WARNING      = 2
	, CONSOLE_LOG_ERROR        = 3
	, CONSOLE_LOG_NONE         = 4
} ConsoleLogLevel;

/**
 ** Debugging is the main backend class for the
 ** debugging functions provided by debug.h
 **
 */
class Debugging : public Singleton<Debugging>, public MessageCallback {
protected:
	inline void printDate() const {
		time_t now = time(nullptr);
		struct tm dt;

		localtime_r(&now, &dt);

		fprintf(stdout, "%2d:%02d:%02d: ", dt.tm_hour, dt.tm_min, dt.tm_sec);
	}

	ConsoleLogLevel consoleLogLevel;

public:
	virtual ~Debugging();

	bool registerDebugOption(
			std::string         option,
			DebuggingOptionType type,
			unsigned char       flags,
			const char*         filename    = 0,
			const char*         description = 0);

	virtual bool messageCallback(
			const std::string               &messageName,
			const google::protobuf::Message &msg,
			int32_t                          id,
			RemoteConnectionPtr             &remote) override;

	/** Retrieve a debug option object.
	 **
	 ** @param option  Name of the debug option
	 **
	 ** @return a pointer to the debug option, or nullptr if no such option was registered
	 */

	inline DebuggingOption* getDebugOption(std::string option) const {
		std::transform(option.begin(), option.end(), option.begin(), ::tolower);
		DebugOptionContainer::const_iterator it = debugging_options.find(option);
		if (it != debugging_options.end())
			return it->second;
		else {
			assert(false);
			return nullptr;
		}
	}

	/** Check whether a debug option is valid (i.e. whether it was registered)
	 **
	 ** @param option  Name of the debug option
	 **
	 ** @return true iff the debug option exists
	 */

	inline bool validDebugOption(std::string option) const {
		std::transform(option.begin(), option.end(), option.begin(), ::tolower);
		return getDebugOption(option) != nullptr;
	}

	/** Check whether a debug option is enabled
	 **
	 ** @param option  Name of the debug option
	 **
	 ** @return true iff debug option is active
	 */

	inline bool activeDebugOption(std::string option) const {
		std::transform(option.begin(), option.end(), option.begin(), ::tolower);
		DebuggingOption *debugOption = getDebugOption(option);
		if (debugOption != nullptr)
			return debugOption->enabled;
		else
			return false;
	}

	/** Set the log level for the console output
	 **
	 ** @param newLogLevel  New log level for the console
	 */

	inline void setConsoleLogLevel(ConsoleLogLevel newLogLevel) {
		consoleLogLevel = newLogLevel;
	}

	/** Print an error message
	 **
	 ** @param format  Format string, followed by parameters (cmp. printf)
	 */

	inline void printError(const char *format, ...) const __attribute__ ((format (printf, 2, 3))) {
		if (consoleLogLevel > CONSOLE_LOG_ERROR)
			return;

		va_list vl;
		va_start(vl, format);
		fprintf(stdout, "\033[31m");
		printDate();
		vfprintf(stdout, format, vl);
		fprintf(stdout, "\033[0m\n");
		va_end(vl);
	}

	/** Print a warning message
	 **
	 ** @param format  Format string, followed by parameters (cmp. printf)
	 */

	inline void printWarning(const char *format, ...) const __attribute__ ((format (printf, 2, 3))) {
		if (consoleLogLevel > CONSOLE_LOG_WARNING)
			return;
		va_list vl;
		va_start(vl, format);
		fprintf(stdout, "\033[35m");
		printDate();
		vfprintf(stdout, format, vl);
		fprintf(stdout, "\033[0m\n");
		va_end(vl);
	}

	/** Print an info message
	 **
	 ** @param format  Format string, followed by parameters (cmp. printf)
	 */

	inline void printInfo(const char *format, ...) const __attribute__ ((format (printf, 2, 3))) {
		if (consoleLogLevel > CONSOLE_LOG_INFO)
			return;
		va_list vl;
		va_start(vl, format);
		printDate();
		vfprintf(stdout, format, vl);
		fprintf(stdout, "\n");
		va_end(vl);
	}

	/** Send debug message of type TEXT
	 **
	 ** @param option    Debug option
	 ** @param format    Format string, followed by optional parameters (cmp. printf)
	 */

	inline void sendDebugText(const DebuggingOption *option, const char *format, ...) const __attribute__ ((format (printf, 3, 4))) {
		if (option && option->enabled) {
			va_list vl;
			va_start(vl, format);

			// output to commandline if enabled
			if (cmdlineOutDebugOption(option)) {
				vfprintf(stdout, format, vl);
				fprintf(stdout, "\n");
			}

			// output only if network output is enabled
			if (option->netout) {
				processMessageOut(option, format, vl);
			}
			va_end(vl);
		}
	}

	/** Send debug message of type PLOTTER
	 **
	 ** @param option
	 ** @param series
	 ** @param x
	 ** @param y
	 */

	inline void sendDebugPlot(const DebuggingOption *option, const std::string &series, double x, double y) const {
		if (option && option->enabled)
			processMessageOut(option, series, x, y);
	}

	/** Send debug message of type PLOTTER
	 **
	 ** @param option
	 ** @param series
	 ** @param x
	 ** @param y
	 */

	inline void sendDebugPlot(const DebuggingOption *option, const std::string &series, Millisecond x, double y) const {
		if (option && option->enabled)
			processMessageOut(option, series, x.value(), y);
	}

	/** Send debug message of type 3D
	 **
	 ** @param option
	 ** @param series
	 ** @param x
	 ** @param y
	 */

	inline void sendDebug3D(const DebuggingOption *option, const de::fumanoids::message::Debug3D &commands) const {
		if (option && option->enabled)
			processMessageOut(option, commands);
	}

	/** Send debug message of type TABLE
	 **
	 ** @param option
	 ** @param key
	 ** @param payload
	 */

	inline void sendDebugTable(const DebuggingOption *option, const std::string &key, const std::string &payload) const {
		if (option && option->enabled)
			processMessageOut(option, key, payload);
	}

	/** Send debug message of type TABLE
	 **
	 ** @param option
	 ** @param key
	 ** @param payload
	 */

	inline void sendDebugTable(const DebuggingOption *option, const std::string &key, const double payload) const {
		if (option && option->enabled)
			processMessageOut(option, key, payload);
	}

	uint32_t registerImage(const std::string &imageName);

	/** Retrieves a reference to the ImageDebugger instance
	 ** which can be used to add draw debug statements.
	 **
	 ** @return reference to the ImageDebugger instance
	 */
	inline ImageDebugger& getImageDebugger(const uint32_t imageID=0) const {
		assert(imageDebuggers.size() > imageID);
		return *imageDebuggers[imageID];
	}

	/** Retrieves a reference to the ImageDebugger instance
	 ** which can be used to add draw debug statements.
	 **
	 ** @return reference to the ImageDebugger instance
	 */
	inline ImageDebugger& getImageDebugger(const std::string &name) const {
		assert(registeredImages.find(name) != registeredImages.end());
		return getImageDebugger( registeredImages.find(name)->second );
	}

	void sendImageDebug(int frameNumber);

private:
	Debugging();
	friend class Singleton<Debugging>;

	CriticalSection cs;

	std::map<std::string, uint32_t> registeredImages;
	std::vector<ImageDebugger *> imageDebuggers;

	typedef std::map<std::string, DebuggingOption*const> DebugOptionContainer;
	DebugOptionContainer debugging_options;

	void switchOption(const std::string &option, bool status);

	void processMessageOut(const DebuggingOption *option, const char *format, va_list &vl) const;
	void processMessageOut(const DebuggingOption *option, const char *payload) const;
	void processMessageOut(const DebuggingOption *option, const std::string &series, double x, double y) const;
	void processMessageOut(const DebuggingOption *option, const std::string &series, double x, double y, double z) const;
	void processMessageOut(const DebuggingOption *option, const std::string &key, const std::string &payload) const;
	void processMessageOut(const DebuggingOption *option, const std::string &key, double value) const;
	void processMessageOut(const DebuggingOption *option, const de::fumanoids::message::Debug3D &commands) const;

	void sendMessage(de::fumanoids::message::Message &msg) const;

	/** Checks whether a debug option should also be printed on console (if enabled)
	 **
	 ** @param option  Debug option
	 **
	 ** @return true iff option should be printed on console
	 */

	inline bool cmdlineOutDebugOption(const DebuggingOption *option) const {
		if (consoleLogLevel != CONSOLE_LOG_PERMIT_ALL)
			return false;

		return option->cmdlineout;
	}

	/** Enable a debug option
	 **
	 ** @param option  Debug option
	 */

	inline void enableDebugOption(const std::string &option) {
		DebuggingOption *ptr = getDebugOption(option);
		if (ptr != nullptr) {
			ptr->enabled = true;
		}
	}

	/** Disable a debug option
	 **
	 ** @param option  Debug option
	 */

	inline void disableDebugOption(const std::string &option) {
		DebuggingOption *ptr = getDebugOption(option);
		if (ptr != nullptr) {
			ptr->enabled = false;
		}
	}
};

/**
 * @}
 */

#endif /* _DEBUGGING_H_ */
