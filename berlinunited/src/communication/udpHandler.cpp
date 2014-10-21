/*
 * udpHandler.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: dseifert
 */

#include "udpHandler.h"
#include "remoteUDPConnection.h"
#include "commHandlerManager.h"

#include "platform/system/transport/transport_udp.h"

#include "debug.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

UDPHandler::UDPHandler(CommHandlerManager *manager)
	: CommHandler(manager)
	, cs()
	, transport(0)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

UDPHandler::~UDPHandler() {
	cancel();

	if (transport) {
		transport->close();
		delete transport;
		transport = 0;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

void UDPHandler::init(int localPort, int broadcastPort) {
	transport = new TransportUDP(localPort, broadcastPort, true );
	if (transport->open() == false) {
		ERROR("Error opening UDP handler transport from port %d to port %d", localPort, broadcastPort);
		return;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

void UDPHandler::threadMain() {
	while (isRunning()) {
		if (transport->waitForData(1, Microsecond(500*milliseconds))) {
			RemoteUDPConnection* process = new RemoteUDPConnection(transport);
			manager->process( RemoteConnectionPtr(process)  );
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

RemoteConnection *UDPHandler::getBroadcastConnection() {
	RemoteUDPConnection *cnc = new RemoteUDPConnection(transport);

	cnc->setToBroadcast();
	return cnc;
}
