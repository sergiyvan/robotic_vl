#include "remoteUDPConnection.h"
#include "commHandlerManager.h"

#include "debug.h"
#include "services.h"
#include "management/config/config.h"

#include "platform/system/transport/transport_udp.h"

#include <stdio.h>


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

RemoteUDPConnection::RemoteUDPConnection(TransportUDP *transport)
	: cs()
	, broadcasting(false)
	, remoteAddress()
	, transport(transport)
	, buffer(NULL)
	, bufferIndex(0)
	, bufferSize(0)
{
	if (transport->waitForData(1, 100*microseconds)) {
		buffer = new uint8_t[BUFSIZE];
		bufferSize = transport->read(buffer, BUFSIZE, &remoteAddress); // TODO: may block
	}

	cs.setName("RemoteUDPConnection::cs");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

RemoteUDPConnection::~RemoteUDPConnection() {
	if (buffer) {
		delete[] buffer;
		buffer = NULL;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool RemoteUDPConnection::send(void const* data, uint32_t dataLength) {
	if (data == 0)
		return false;
	else if (dataLength > 65000) {
		ERROR("Message is too large for UDP");
		return false;
//	} else if (dataLength > 16394) {
//		// the larger a message, the more likely is a collision and packet loss
//		WARNING("Message is a bit large (%d bytes) for efficient transport", dataLength);
	} else if (false == transport->isConnected()) {
		fprintf(stderr, "Trying to send through disconnected UDP transport\n");
		return false;
	}

	// actually send data
	cs.enter();
	int bytesWritten = transport->write(data, dataLength, broadcasting ? NULL : &remoteAddress);
	if (bytesWritten < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			fprintf(stderr, "UDP socket busy (buffer full), could not send %d bytes\n", dataLength);
		else
			fprintf(stderr, "error sending message: %s\n", strerror(errno));
	}
	//	else
//		totalBytesSent += bytesWritten;
	cs.leave();

	return (bytesWritten == (signed)dataLength);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

void RemoteUDPConnection::setToBroadcast() {
	std::string remoteIP = services.getConfig().get<std::string>("comm.remote.ip");

	// if we have a remote IP, we do not broadcast but instead send broadcasts just to this IP
	if (remoteIP != "") {
		int localPort  = services.getConfig().get<uint16_t>("comm.port");
		int remotePort = services.getConfig().get<uint16_t>("comm.remote.port", localPort);

		remoteAddress.sin_family      = AF_INET;
		remoteAddress.sin_addr.s_addr = inet_addr(remoteIP.c_str());
		remoteAddress.sin_port        = htons(remotePort);
	} else {
		broadcasting = true;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool RemoteUDPConnection::isConnected() {
	return transport != NULL && transport->isConnected();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool RemoteUDPConnection::connect() {
	if (isConnected() || nullptr == transport)
		return false;

	return transport->open();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

int32_t RemoteUDPConnection::read(uint8_t *data, uint32_t dataLength) {
	if (buffer == 0)
		return -1;

	uint32_t remainingData = std::max (0, (int32_t)( bufferSize - bufferIndex));
//	printf("bs %d bi %d dl %d rd %d\n", bufferSize, bufferIndex, dataLength, remainingData);
	int32_t dataCount = std::min(dataLength, remainingData);
	if (dataCount > 0) {
		memcpy(data, buffer + bufferIndex, dataCount);
		bufferIndex += dataCount;
	}

	return dataCount;
}
