#include "transport_tcp_server.h"
#include "transport_tcp.h"
#include "platform/system/timer.h"
#include "debug.h"

#include <arpa/inet.h>


/*------------------------------------------------------------------------------------------------*/

TransportTCPServer::TransportTCPServer(int _port)
	: listenSock(-1)
	, port(_port)
{
}

TransportTCPServer::~TransportTCPServer() {
	close();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 ** @return FALSE if connection could not be established
 */

bool TransportTCPServer::init() {
	// create socket
	if ((listenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		ERROR("Failure creating TCP socket on port %d", port);
		listenSock = 0;
		// TODO: handle error - failure creating socket
		return false;
	}

	// enable address reuse
	int reuse = 1;
	setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	// construct sockaddr_in structure
	struct sockaddr_in sa = { 0 };
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port        = htons(port);

	// bind socket
	unsigned int saLen = sizeof(sa);
	if ( ::bind(listenSock, (struct sockaddr *) &sa, saLen) < 0 ) {
//		ERROR("Failure binding TCP socket to port %d", port);
		::close(listenSock);
		listenSock = 0;
		return false;
	}

	// listen on socket
	if (listen(listenSock, 10) < 0 ) {
		ERROR("Error calling listen for TCP socket on port %d", port);
		::close(listenSock);
		listenSock = 0;
		return false;
	}

//	INFO("TCP server started listening on port %d", port);
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

void TransportTCPServer::close() {
	if (listenSock != -1) {
		::shutdown(listenSock, SHUT_RDWR);
		::close(listenSock);
	}

	listenSock = -1;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

TransportTCP *TransportTCPServer::waitForIncoming(Microsecond timeout) {
	if (listenSock < 0)
		return 0;

	// wait for incoming request
	struct timeval tvTimeout = { 0, (__suseconds_t)timeout.value() };
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(listenSock, &readfds);

	int s = select(listenSock+1, &readfds, 0, 0, &tvTimeout);
	if (s <= 0)
		return 0; // timeout (or error)

	// accept incoming connection
	struct sockaddr addr;
	socklen_t addrlen = sizeof addr;
	int connectionSocket = accept(listenSock, &addr, &addrlen);
	if (connectionSocket < 0) {
		ERROR("Error calling accept() for server listening on port %d", port);
		return 0;
	}

	return new TransportTCP(connectionSocket, (struct sockaddr_in&)addr);
}
