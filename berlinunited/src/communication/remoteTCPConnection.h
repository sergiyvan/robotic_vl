/*
 * remoteTCPConnection.h
 *
 *  Created on: Nov 28, 2011
 *      Author: dseifert
 */

#ifndef REMOTETCPCONNECTION_H_
#define REMOTETCPCONNECTION_H_

#include "remoteConnection.h"

#include <limits.h>


// forward declaration
class TransportTCP;


/*------------------------------------------------------------------------------------------------*/

/** Representation of a remote TCP connection
 */
class RemoteTCPConnection : public RemoteConnection {
public:
	RemoteTCPConnection(TransportTCP *transport);
	virtual ~RemoteTCPConnection();

	virtual bool    isConnected();
	virtual bool    connect();

	virtual int32_t read(uint8_t    *data, uint32_t dataLength);
	virtual bool    send(void const *data, uint32_t dataLength);

	virtual uint32_t getMaxPackageSize() {
		return UINT_MAX;
	}

private:
	TransportTCP *transport;

	// we are using pointers, it does not make sense to copy this handler so prevent it
	RemoteTCPConnection(const RemoteTCPConnection &) = delete;
	RemoteTCPConnection& operator=(const RemoteTCPConnection &) = delete;
};

#endif
