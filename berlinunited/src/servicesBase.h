#ifndef SERVICESBASE_H_
#define SERVICESBASE_H_

#include "management/config/configRegistry.h"
#include "platform/system/events.h"
#include "platform/statusOutput.h"
#include "utils/patterns/singleton.h"

#include "ModuleFramework/ModuleManager.h"
#include "ModuleFramework/moduleManagers.h"

#include "communication/messageRegistry.h"

#include <inttypes.h>

#include <string>


/*------------------------------------------------------------------------------------------------*/

// events we provide
DECLARE_EVENT_ID(EVT_CONFIGURATION_LOADED);
DECLARE_EVENT_ID(EVT_BEFORE_CONFIG_SAVE);
DECLARE_EVENT_ID(EVT_IMAGE_CAPTURED)



/*------------------------------------------------------------------------------------------------*/

// forward declarations for services, to avoid having to
// make this a huge include-fest
class Comm;
class Config;
class Events;
class MessageRegistry;
class StatusOutput;

class ServiceInitStructure;

/*------------------------------------------------------------------------------------------------*/

typedef std::map<std::string, int> ModuleManagerStatus;


/*------------------------------------------------------------------------------------------------*/

/** The ServicesBase class is the center of the framework. It is in charge of
 ** setting up the required elements provided by the framework. As downstream
 ** projects will add their own special functionality, it is assumed that a
 ** class named Services will derive from ServicesBase class and be used to
 ** access it.
 */

class ServicesBase {
protected:
	// constructor
	ServicesBase();

	// no copy constructor
	ServicesBase(const ServicesBase &) = delete;

	// no assignment operator
	ServicesBase & operator=(const ServicesBase &) = delete;

public:
	virtual ~ServicesBase();

	/** Create a new configuration object of the given type C and set it as
	 ** the global configuration (handling network requests and using the
	 ** options registered in ConfigRegistry).
	 **
	 ** @param configurationFileName   Name of configuration file to load
	 */
	template <typename C>
	void setupConfig(const std::string &configurationFileName = "") {
		if (nullptr != config)
			throw std::runtime_error("Config is already set up.");

		// create instance and copy the registered options into it
		config = new C();
		ConfigRegistry::getInstance().copyTo(*config);

		if (!configurationFileName.empty()) {
			if (false == config->load(configurationFileName))
				WARNING("Configuration file not found/loaded.");
		}

		// register for configuration requests
		getMessageRegistry().registerMessageCallback(config, "configurationRequest");
	}

	// initialize framework
	virtual bool init(int argc, char* argv[], ServiceInitStructure* sis = nullptr);

	// run framework
	virtual int run();

	// run managers
	virtual void runManagers();
	virtual void runManagers(const ModuleManagersState *state);


	/** Terminate the application. Must not be called from threads.
	 **
	 ** @param notifyOfTermination
	 */
	virtual void terminate(bool notifyOfTermination=true);


	/** Trigger the termmination of the application. This is the function
	 ** that should be called outside of main loops (e.g. outside of
	 ** command line callbacks).
	 */

	static void triggerTermination();
	static void waitForTermination();

public:
	// accessors
	inline Comm&            getComm()            { return *comm;           }
	inline Config&          getConfig()          { return *config;         }
	inline Events&          getEvents()          { return *events;         }
	inline MessageRegistry& getMessageRegistry() { return messageRegistry; }
	inline ModuleManagers&  getModuleManagers()  { return moduleManagers;  }
	inline StatusOutput&    getStatusOutput()    { return StatusOutput::getInstance(); }

	/// return id of this robot
	inline int16_t getID() const { return id; }

	/// return name of this robot
	inline const std::string& getName() const { return name; }

	/// return time the software has started running
	inline robottime_t getStarttime() const { return startTime; }

	/** Return time (duration) this robot software has been running
	 **
	 ** @return uptime in milliseconds
	 */
	inline Millisecond getUptime() const { return (getCurrentTime() - startTime); }


protected:
	Comm            *comm;            /// the communication manager
	Config          *config;          /// the configuration manager
	Events          *events;          /// the events manager
	MessageRegistry  messageRegistry; /// message callback registry
	ModuleManagers  &moduleManagers;  /// module manager activator

	int16_t          id;              /// id of this robot (valid values > 0)
	std::string      name;            /// name of this robot
	robottime_t      startTime;       /// time the software started running

	/// termination event, triggers when robot is supposed to shut down
	static Event terminationEvent;

	CriticalSection cs;
};

#endif
