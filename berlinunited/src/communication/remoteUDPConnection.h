#ifndef REMOTEUDPCONNECTION_H_
#define REMOTEUDPCONNECTION_H_

#include "remoteConnection.h"
#include "platform/system/thread.h"

#include <arpa/inet.h>

// forward declarations
class TransportUDP;


/*------------------------------------------------------------------------------------------------*/

/** RemoteUDPConnection is an established "connection" for a UDP request. It is a rather peculiar
 ** RemoteConnection object, as it is not actually a connection. I.e. it does not support to receive
 ** additional data but can only be used to send data out.
 */

class RemoteUDPConnection : public RemoteConnection {
public:
	RemoteUDPConnection(TransportUDP *transport);
	virtual ~RemoteUDPConnection();

	void setToBroadcast();

	virtual bool    isConnected();
	virtual bool    connect();

	virtual int32_t read(uint8_t    *data, uint32_t dataLength);
	virtual bool    send(void const *data, uint32_t dataLength);

	virtual uint32_t getMaxPackageSize() {
		return 65000;
	}

private:
	CriticalSection cs;

	bool broadcasting;
	struct sockaddr_in  remoteAddress;

	TransportUDP       *transport;

	// data
#define BUFSIZE 65536
	uint8_t  *buffer;
	int32_t   bufferIndex;
	int32_t   bufferSize;

	// we are using pointers, it does not make sense to copy this handler so prevent it
	RemoteUDPConnection(const RemoteUDPConnection &) = delete;
	RemoteUDPConnection& operator=(const RemoteUDPConnection &) = delete;
};

#endif
