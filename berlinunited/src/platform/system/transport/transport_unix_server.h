#ifndef __UNIXSERVER_H__
#define __UNIXSERVER_H__

#include "transport_server.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The Unix Server listens on a socket path and waits for incoming requests. Upon connection it will
 ** create a TransportUnix object.
 **
 ** @ingroup transport
 **
 */

class TransportUnixServer : public TransportServer{
protected:
	/// listening socket
	int listenSock;

	/// path
	std::string location;

public:
	TransportUnixServer(const char* location);
	virtual ~TransportUnixServer();

	/// setup listening process
	virtual bool init();

	/// listen for connection
	virtual Transport* waitForIncoming(Microsecond timeout);

	/// clean up
	virtual void close();
};

#endif
