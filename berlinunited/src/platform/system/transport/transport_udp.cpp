#include "transport_udp.h"
#include "platform/system/timer.h"

#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <ifaddrs.h>
#include <sstream>
#include <stdlib.h>
#include <fcntl.h>


/*------------------------------------------------------------------------------------------------*/

TransportUDP::TransportUDP(int _port, int _remotePort, bool _broadcasting)
	: sock(-1)
	, port(_port)
	, remotePort(_remotePort)
	, broadcasting(_broadcasting)
	, lastInterfaceScan(0)
	, cs()
	, broadcastAddresses()
{
	std::stringstream csNameS;
	csNameS  << "TransportUDP on port " << port;
	cs.setName(csNameS.str());

	if (remotePort == 0)
		remotePort = port;
}

TransportUDP::~TransportUDP() {
	for (uint32_t i=0; i < broadcastAddresses.size(); i++)
		free(broadcastAddresses[i]);

	close();
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 ** @return FALSE if connection could not be established
 */

bool TransportUDP::open() {
	// create socket
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "\n\n=======================\nfailure creating socket\n\n\n");
		sock = 0;
		// TODO: handle error - failure creating socket
		return false;
	}

	// dis/enable broadcast
	int broadcastPermission = (broadcasting ? 1 : 0);
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission));

	// make socket non-blocking
	int fcntl_flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, fcntl_flags | O_NONBLOCK);

	// construct sockaddr_in structure
	struct sockaddr_in sa = { 0 };
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port        = htons(port);

	// bind socket
	unsigned int saLen = sizeof(sa);
	if ( ::bind(sock, (struct sockaddr *) &sa, saLen) < 0 ) {
		fprintf(stderr, "\n\n======================\nfailure binding socket to port %d\n\n\n", port);
		::close(sock);
		sock = 0;
		return false;
	}

	// collect interfaces (needed for broadcast)
	collectInterfaces();

//	printf("UDP connection on port %d successfully established\n", port);
	return true;
}

/*------------------------------------------------------------------------------------------------*/

bool TransportUDP::openWithoutBind() {
	// create socket
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "\n\n=======================\nfailure creating socket\n\n\n");
		sock = 0;
		// TODO: handle error - failure creating socket
		return false;
	}

//	printf("UDP connection on port %d successfully established\n", port);
	return true;
}


/*------------------------------------------------------------------------------------------------*/

void TransportUDP::collectInterfaces() {
	CriticalSectionLock lock(cs);

	// free current addresses
	for (uint32_t i=0; i < broadcastAddresses.size(); i++)
		free(broadcastAddresses[i]);

	broadcastAddresses.clear();

	struct ifaddrs * ifap;
	if (getifaddrs(&ifap) == 0) {

		// go through all interfaces
		struct ifaddrs * p = ifap;
		while (p) {

			// if we got an address for IP, use this interface
			if (p->ifa_addr && p->ifa_addr->sa_family == AF_INET) {
				// we make an intermediary cast to void* to prevent warnings about alignment requirements
				uint32_t interfaceAddress = ntohl(((struct sockaddr_in *)(void*)p->ifa_addr)->sin_addr.s_addr);
				uint32_t broadcastAddress = ((struct sockaddr_in *)(void*)p->ifa_broadaddr)->sin_addr.s_addr;

#if defined DESKTOP
				// use only local interfaces (127.0.0.1) or private address ranges, will also prevent to broadcast
				// outside a virtual or local network
				if (
				       (interfaceAddress > 0x0A000000 && interfaceAddress < 0x0AFFFFFF)  // 10.x.x.x
				    || (interfaceAddress > 0xAC0F0000 && interfaceAddress < 0xAC1FFFFF)  // 172.16.x.x through 172.31.x.x
				    || (interfaceAddress > 0xC0A80000 && interfaceAddress < 0xC0A8FFFF)  // 192.168.x.x
				    || (interfaceAddress > 0x7F000000 && interfaceAddress < 0x7F0000FF)  // 127.0.0.x
				   )
				{
#else
				// use all interfaces except the local one (we do not want to send to ourselves!)
				if (interfaceAddress > 0 && interfaceAddress != 0x7F000001) {
#endif
//					printf("%s: broadcasting\n", p->ifa_name);
					struct sockaddr_in *recipient = (sockaddr_in*) malloc(sizeof(struct sockaddr_in));
					if (recipient) {
						memset(recipient, 0, sizeof(struct sockaddr_in));
						recipient->sin_family = AF_INET;
						recipient->sin_port = htons(remotePort);
						recipient->sin_addr.s_addr = broadcastAddress;

						broadcastAddresses.push_back(recipient);
					} else
						printf("\033[31mERROR allocating memory for recipient structure\033[0m\n");
				} else {
//					printf("%s: NOT broadcasting\n", p->ifa_name);
				}
			}
			p = p->ifa_next;
		}
		freeifaddrs(ifap);
	}

	lastInterfaceScan = getCurrentTime();
}


/*------------------------------------------------------------------------------------------------*/

/** Check whether connection was established
 **
 **
 ** @return
 */

bool TransportUDP::isConnected() {
	return sock != -1;
}


/*------------------------------------------------------------------------------------------------*/

/** close connection
 **
 **
 */

void TransportUDP::close() {
	if (sock != -1)
		::close(sock);

	sock = -1;
}


/*------------------------------------------------------------------------------------------------*/

/** Send a broadcast message
 **
 **
 */

int TransportUDP::write(const void *data, uint32_t count) {
	return write(data, count, 0);
}


/*------------------------------------------------------------------------------------------------*/

/** Send a broadcast message
 **
 **
 */

int TransportUDP::write(const void *data, uint32_t count, struct sockaddr_in *recipient) {
	if (sock < 0)
		return 0;

	CriticalSectionLock lock(cs);
	const uint8_t *toSend = (const uint8_t*)data;

	// if no recipient is specified, we assume a broadcast message
	if (recipient == 0) {
		if (lastInterfaceScan + Millisecond(15*seconds) < getCurrentTime())
			collectInterfaces();

		int written = 0;
		for (uint32_t i=0; i < broadcastAddresses.size(); i++) {
			written = std::max(written, write(toSend, count, broadcastAddresses[i]));
		}

		return written;

	} else {
		// send data directly to recipient
		return sendto(sock, data, count, 0, (const sockaddr*)recipient, sizeof *recipient);
	}
}


/*------------------------------------------------------------------------------------------------*/

/** Read data
 **
 **
 */

int TransportUDP::read(void *data, uint32_t count) {
	struct sockaddr_in remoteAddress;
	return read(data, count, &remoteAddress);
}


/*------------------------------------------------------------------------------------------------*/

/** Read data
 **
 **
 */

int TransportUDP::read(void *data, uint32_t count, struct sockaddr_in *remoteAddressP) {
	socklen_t addressLen = sizeof(*remoteAddressP);
	return recvfrom(sock, data, count, 0, (struct sockaddr *) remoteAddressP, &addressLen);
}


/*------------------------------------------------------------------------------------------------*/

/** Wait for data
 **
 **
 */

bool TransportUDP::waitForData(uint32_t, Microsecond timeout) {
	struct timeval tvTimeout = { 0, (__suseconds_t)timeout.value() };
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);

	int s = select(sock+1, &readfds, 0, 0, &tvTimeout);
	if (s <= 0)
		return false; // timeout (or error)
	else
		return true;  // data available
}
