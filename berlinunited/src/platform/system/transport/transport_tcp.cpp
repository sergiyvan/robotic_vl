#include "transport_tcp.h"
#include "platform/system/timer.h"
#include "debug.h"

#include <arpa/inet.h>

CriticalSection TransportTCP::globalCS;


/*------------------------------------------------------------------------------------------------*/

TransportTCP::TransportTCP(int _port, const std::string &_ip)
	: sock(-1)
	, ip(_ip)
	, port(_port)
{
}

TransportTCP::TransportTCP(int _port, const char* _ip)
	: sock(-1)
	, ip(_ip)
	, port(_port)
{
}

TransportTCP::TransportTCP(int _sock, const struct sockaddr_in &sa)
	: sock(_sock)
	, ip()
	, port()
{
	// lock all occurrences of this constructor as we call inet_ntoa which
	// is not thread-safe
	CriticalSectionLock lock(globalCS);

	port = sa.sin_port;
	ip = inet_ntoa(sa.sin_addr);

//	INFO("TCP connection to %s:%d established", ip.c_str(), port);
}

TransportTCP::~TransportTCP() {
	close();
}


/*------------------------------------------------------------------------------------------------*/

/** Establish connection.
 **
 **
 ** @return true iff connection was successfully established
 */

bool TransportTCP::open() {
	if (sock != -1)
		return true;

	// create socket
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		ERROR("Failure creating TCP socket");
		sock = -1;
		// TODO: handle error - failure creating socket
		return false;
	}

	// construct sockaddr_in structure
	struct sockaddr_in sa = { 0 };
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = inet_addr(ip.c_str());
	sa.sin_port        = htons(port);

	// connect socket
	unsigned int saLen = sizeof(sa);
	if ( connect(sock, (struct sockaddr *) &sa, saLen) != 0 ) {
		ERROR("Failure connecting TCP socket to port %d", port);
		::close(sock);
		sock = -1;
		return false;
	}

//	INFO("TCP connection to %s:%d established", ip.c_str(), port);
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Check connection status
 **
 ** @return true iff connection was established
 */

bool TransportTCP::isConnected() {
	return sock != -1;
}


/*------------------------------------------------------------------------------------------------*/

/** Close connect
 **
 **
 */

void TransportTCP::close() {
	if (sock != -1) {
		::shutdown(sock, SHUT_RDWR);
		::close(sock);
	}

	sock = -1;
}


/*------------------------------------------------------------------------------------------------*/

/** Send data.
 **
 ** @param data   Data to send
 ** @param count  Size (in bytes) of data
 **
 ** @return number of bytes written, negative numbers are inverted error codes
 */

int TransportTCP::write(const void *data, uint32_t count) {
	robottime_t    lastTime     = getCurrentTime();
	uint32_t       bytesWritten = 0;
	const uint8_t *toSend = (const uint8_t*) data;

	if (sock == -1)
		return -EBADF;

	// write data out as long as we have some left and as long as
	// the last written data did not went out more than 0.5s ago
	do {
		int chunkSize = count - bytesWritten;
		if (chunkSize > 65536) chunkSize = 65536;

		ssize_t w = ::send(sock, toSend + bytesWritten, chunkSize, MSG_NOSIGNAL);

		if (w == -1) {
			if (errno == EBADF || errno == ECONNRESET || errno == ENOTCONN) {
				WARNING("TCP Socket shutdown due to error %s (0x%x) while writing", strerror(errno), errno);
				close();
			}
			break;
		} else if (w > 0) {
			lastTime = getCurrentTime();
			bytesWritten += w;
		}
	} while (bytesWritten < count && (getCurrentTime() - lastTime < 500*milliseconds) );

	return bytesWritten;
}


/*------------------------------------------------------------------------------------------------*/

/** Read data
 **
 ** @param data    Pointer to memory area to store read data
 ** @param count   Maximum number of bytes to read
 **
 ** @return number of bytes read, but at most 'count' (<0 for error)
 */

int TransportTCP::read(void *data, uint32_t count) {
	uint32_t bytesRead = 0;

	// read data up to 'count' bytes but return when we have not
	// received anything in 0.5s
	uint8_t *toRead = (uint8_t*) data;
	do {
		waitForData(1, Microsecond(500*milliseconds));
		int r = ::recv(sock, toRead + bytesRead, count - bytesRead, MSG_DONTWAIT);
		if (r == -1) {
			if (errno == EBADF || errno == ECONNRESET || errno == ENOTCONN) {
				WARNING("TCP Socket shutdown due to error %s (0x%x) while reading", strerror(errno), errno);
				close();
			}
			break;
		} else if (r == 0) {
			break;
		} else
			bytesRead += r;
	} while (bytesRead < count);

/*
	if (bytesRead > 0) {
		printf("read %d bytes: ", bytesRead);
		for (int i=0; i < bytesRead; i++)
			printf("%02x ", data[i]);
		printf("\n");
	}
*/
	return bytesRead;
}


/*------------------------------------------------------------------------------------------------*/

/** wait for data
 **
 ** @param microsecTimeout  Timeout in micro seconds
 **
 ** @return true iff data is available for reading
 */

bool TransportTCP::waitForData(uint32_t, Microsecond timeout) {
	struct timeval tvTimeout = { 0, (__suseconds_t)timeout.value() };
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);

	int s = select(sock + 1, &readfds, 0, 0, &tvTimeout);
	if (s <= 0) {
		if (errno == EBADF || errno == ECONNRESET)
			this->close();

		return false; // timeout (or error)
	} else
		return true;  // data available
}
