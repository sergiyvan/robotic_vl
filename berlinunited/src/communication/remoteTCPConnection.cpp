#include "remoteTCPConnection.h"
#include "commHandlerManager.h"

#include "platform/system/transport/transport_tcp.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

RemoteTCPConnection::RemoteTCPConnection(TransportTCP *transport)
	: transport(transport)
{

}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

RemoteTCPConnection::~RemoteTCPConnection() {
	transport->close();
	delete transport;
	transport = 0;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool RemoteTCPConnection::send(void const* data, uint32_t dataLength) {
	return (signed)dataLength == transport->write((uint8_t const*)data, dataLength);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool RemoteTCPConnection::isConnected() {
	return transport && transport->isConnected();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool RemoteTCPConnection::connect() {
	if (isConnected() || nullptr == transport)
		return false;

	return transport->open();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

int32_t RemoteTCPConnection::read(uint8_t *data, uint32_t dataLength) {
	return transport->read(data, dataLength);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

