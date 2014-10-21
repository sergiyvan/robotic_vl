#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "transport_server.h"
#include "transport_tcp.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The TCP Server listens on a port and waits for incoming requests. Upon connection it will
 ** create a TransportTCP object.
 **
 ** @ingroup transport
 **
 */

class TransportTCPServer : public TransportServer {
protected:
	/// listening socket
	int listenSock;

	/// port number
	int port;

public:
	TransportTCPServer(int port);
	virtual ~TransportTCPServer();

	/// setup listening process
	virtual bool init();

	/// listen for connection
	virtual TransportTCP* waitForIncoming(Microsecond timeout);

	/// clean up
	virtual void close();
};

#endif
