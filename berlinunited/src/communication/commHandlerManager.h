#ifndef COMMHANDLERMANAGER_H_
#define COMMHANDLERMANAGER_H_

#include "remoteConnection.h"


/** CommHandlerManager is an interface (ABC) for instances that manage and coordinate communication.
 */
class CommHandlerManager {
public:
	virtual ~CommHandlerManager() {}

	virtual bool process(RemoteConnectionPtr remote) = 0;
};

#endif
