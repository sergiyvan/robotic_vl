/*
 * tcpHandler.h
 *
 *  Created on: Nov 26, 2011
 *      Author: dseifert
 */

#ifndef TCPHANDLER_H_
#define TCPHANDLER_H_

#include "commHandler.h"

#include "platform/system/transport/transport_tcp.h"
#include "platform/system/transport/transport_tcp_server.h"


class TCPHandler : public CommHandler {
public:
	TCPHandler(CommHandlerManager *manager);
	virtual ~TCPHandler();

	virtual const char* getName() const override {
		return "TCPHandler";
	}

	virtual void init(int port, int unusedBroadcastPort=0) override;
	virtual void threadMain() override;

protected:
	///
	TransportTCPServer *server;
	TransportTCP       *transport;

	/// critical section
	CriticalSection cs;

	// we are using pointers, it does not make sense to copy this handler so prevent it
	TCPHandler(const TCPHandler &) = delete;
	TCPHandler& operator=(const TCPHandler &) = delete;
};

#endif /* TCPHANDLER_H_ */
