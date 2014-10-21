/*
 * tcpHandler.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: dseifert
 */

#include "tcpHandler.h"
#include "remoteTCPConnection.h"
#include "commHandlerManager.h"
#include "debug.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

TCPHandler::TCPHandler(CommHandlerManager *manager)
	: CommHandler(manager)
	, server(0)
	, transport(0)
	, cs()
{
	cs.setName("TCPHandler::cs");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

TCPHandler::~TCPHandler() {
	cancel();

	// close transport
	if (server) {
		server->close();
		delete server;
	}

	server = 0;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Initialization.
**/

void TCPHandler::init(int port, int) {
	CriticalSectionLock lock(cs);

	server = new TransportTCPServer(port);
	if (server->init() == false)
		return;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

void TCPHandler::threadMain() {
	while (isRunning()) {
		TransportTCP *transport = server->waitForIncoming(Microsecond(500*milliseconds));
		if (transport) {
			manager->process(RemoteConnectionPtr(new RemoteTCPConnection(transport)));
		}
	}
}
