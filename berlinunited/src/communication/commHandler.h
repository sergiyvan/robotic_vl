
#ifndef COMMHANDLER_H_
#define COMMHANDLER_H_

#include "platform/system/thread.h"

// forward declaration
class CommHandlerManager;


/*------------------------------------------------------------------------------------------------*/

/**
 ** A comm handler is an object that handles incoming communication on a specific transport channel.
 ** It is waiting for incoming connections and creates a corresponding RemoteConnection object.
 **
 ** The CommHandler class is an interface (ABC) and needs to be specialized.
 */

class CommHandler : public Thread {
public:
	CommHandler(CommHandlerManager *manager);
	virtual ~CommHandler();

	virtual void init(int port, int broadcastPort=0) = 0;

	// Overloaded copy constructor. A copy of the comm handler
	// would still use the same CommHandlerManager, so we keep with
	// the shallow copy.
	CommHandler(const CommHandler& other)
		: manager(other.manager)
	{
	}

	// Overloaded assignment operator. A copy of the comm handler
	// would still use the same CommHandlerManager, so we keep with
	// the shallow copy.
	CommHandler& operator=(const CommHandler& other) {
		this->manager = other.manager;
		return *this;
	}

protected:
	CommHandlerManager *manager;
};

#endif /* COMMHANDLER_H_ */
