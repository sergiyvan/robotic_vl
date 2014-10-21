#ifndef __TCPTRANSPORT_H__
#define __TCPTRANSPORT_H__

#include "transport.h"
#include "platform/system/thread.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The TCP transport implements communication via a TCP socket.
 **
 ** @ingroup transport
 **
 */

class TransportTCP : public Transport {
protected:
	/// file descriptor for the TCP port
	int sock;

	/// remote ip address
	std::string ip;

	/// port number
	int port;

	static CriticalSection globalCS;

public:
	TransportTCP(int port, const std::string &ip);
	TransportTCP(int port, const char* ip);
	TransportTCP(int sock, const struct sockaddr_in &sa);
	virtual ~TransportTCP();

	/// open connection
	virtual bool open();

	/// returns whether connection to bus was established
	virtual bool isConnected();

	/// close connection
	virtual void close();

	/// write
	virtual int write(const void *data, uint32_t count);

	/// read
	virtual int read(void *data, uint32_t count);

	/// wait for data
	virtual bool waitForData(uint32_t, Microsecond timeout);
};

#endif
