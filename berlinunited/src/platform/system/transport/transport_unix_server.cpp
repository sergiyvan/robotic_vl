#include "transport_unix_server.h"
#include "transport_unix.h"
#include "platform/system/timer.h"
#include "debug.h"

#include <arpa/inet.h>


/*------------------------------------------------------------------------------------------------*/

TransportUnixServer::TransportUnixServer(const char* _location)
	: listenSock(0)
	, location(_location)
{
}

TransportUnixServer::~TransportUnixServer() {
	close();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 ** @return FALSE if connection could not be established
 */

bool TransportUnixServer::init() {
	// create socket
	if ((listenSock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		ERROR("Failure creating Unix socket for location %s", location.c_str());
		listenSock = 0;
		// TODO: handle error - failure creating socket
		return false;
	}

	struct sockaddr_un su = { 0 }; // construct sockaddr_un structure
	su.sun_family = AF_UNIX;
	strcpy(su.sun_path, location.c_str());
	unlink(su.sun_path);
	int suLen = strlen(su.sun_path) + sizeof(su.sun_family);

	// bind socket
	if ( ::bind(listenSock, (struct sockaddr *) &su, suLen) < 0 ) {
		ERROR("Failure binding Unix socket to location %s", location.c_str());
		::close(listenSock);
		listenSock = 0;
		return false;
	}

	// listen on socket
	if (listen(listenSock, 10) < 0 ) {
		ERROR("Error calling listen for Unix socket to location %s", location.c_str());
		::close(listenSock);
		listenSock = 0;
		return false;
	}

	INFO("Unix socket server started listening to location %s", location.c_str());
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

void TransportUnixServer::close() {
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

Transport *TransportUnixServer::waitForIncoming(Microsecond timeout) {
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
	int connectionSocket = ::accept(listenSock, NULL, NULL);
	if (connectionSocket < 0) {
		ERROR("Error calling accept() for server listening on location %s", location.c_str());
		return 0;
	}

	return new TransportUnix(connectionSocket, location.c_str());
}
