#ifndef __UDPTRANSPORT_H__
#define __UDPTRANSPORT_H__

#include "transport.h"
#include "platform/system/thread.h"
#include "platform/system/timer.h"

#include <string>
#include <vector>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The UDP transport implements communication via UDP packets
 **
 ** @ingroup transport
 **
 */

class TransportUDP : public Transport {
protected:
	/// sock descriptor for the UDP port
	int sock;

	/// port number
	int port;
	int remotePort;

	/// whether we support broadcasting
	bool broadcasting;

	/// last time for network interface check
	robottime_t lastInterfaceScan;

	CriticalSection cs;

	std::vector<struct sockaddr_in *> broadcastAddresses;

	void collectInterfaces();

public:
	TransportUDP(int port, int remotePort=0, bool broadcasting=true);
	virtual ~TransportUDP();

	/// open connection
	virtual bool open();

	/// open (unidirectional) connection, e.g. write to remote port
	virtual bool openWithoutBind();

	/// returns whether connection to bus was established
	virtual bool isConnected();

	/// close connection
	virtual void close();

	/// retrieve the local port number (data is received on this port, and sent
	/// from this port)
	virtual uint16_t getLocalPort()  { return port;       }

	/// retrieve the remote port number (data is broadcasted to this port)
	virtual uint16_t getRemotePort() { return remotePort; }

	/// write
	virtual int write(const void *data, uint32_t count);
	virtual int write(const void *data, uint32_t count, struct sockaddr_in *recipient);

	/// read
	virtual int read(void *data, uint32_t count);
	virtual int read(void *data, uint32_t count, struct sockaddr_in *remoteAddressP);

	/// wait for data
	virtual bool waitForData(uint32_t, Microsecond timeout);
};

#endif
