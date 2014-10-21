#include "transport_unix.h"
#include "platform/system/timer.h"
#include "debug.h"

#include <arpa/inet.h>

//#define UNIXSOCKETDEBUG


/*------------------------------------------------------------------------------------------------*/

TransportUnix::TransportUnix(const char *_location)
	: sock(-1)
	, location(_location)
{
}

TransportUnix::TransportUnix(int _sock, const char *_location)
	: sock(_sock)
	, location(_location)
{
	INFO("Unix socket connection to %s established", location.c_str());
}

TransportUnix::~TransportUnix() {
	close();
}


/*------------------------------------------------------------------------------------------------*/

/** Establish connection.
 **
 **
 ** @return true iff connection was successfully established
 */

bool TransportUnix::open() {
	if (sock != -1)
		return true;

	// create socket
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		ERROR("Failure creating Unix socket for location %s", location.c_str());
		sock = -1;
		// TODO: handle error - failure creating socket
		return false;
	}

	// construct sockaddr_un structure
	struct sockaddr_un su = { 0 };
	su.sun_family = AF_UNIX;
	strcpy(su.sun_path, location.c_str());

	// connect socket
	int suLen = strlen(su.sun_path) + sizeof(su.sun_family);
	if ( connect(sock, (struct sockaddr *) &su, suLen) != 0 ) {
		ERROR("Failure binding Unix socket to location %s", location.c_str());
		::close(sock);
		sock = -1;
		return false;
	}

	INFO("Unix connection to %s established", location.c_str());
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Check connection status
 **
 ** @return true iff connection was established
 */

bool TransportUnix::isConnected() {
	return sock != -1;
}


/*------------------------------------------------------------------------------------------------*/

/** Close connect
 **
 **
 */

void TransportUnix::close() {
	if (sock != -1)
		::close(sock);

	sock = -1;
}


/*------------------------------------------------------------------------------------------------*/

/** Send data.
 **
 ** @param data   Data to send
 ** @param count  Size (in bytes) of data
 **
 ** @return number of bytes written
 */

int TransportUnix::write(const void *data, uint32_t count) {
	robottime_t    lastTime     = getCurrentTime();
	uint32_t       bytesWritten = 0;
	const uint8_t *toWrite = (const uint8_t*) data;

	if (sock == -1)
		return EBADF;

	// write data out as long as we have some left and as long as
	// the last written data did not went out more than 0.5s ago
	do {
		ssize_t w = ::send(sock, toWrite + bytesWritten, count - bytesWritten, MSG_NOSIGNAL);

		if (w == -1) {
			// TODO: improve error handling
			break;
		} else if (w > 0) {
			lastTime = getCurrentTime();
			bytesWritten += w;
		}
	} while (bytesWritten < count && (getCurrentTime() - lastTime < 500*milliseconds) );

#ifdef UNIXSOCKETDEBUG
	printf("written %d bytes: ", bytesWritten);
	for (unsigned int i=0; i < bytesWritten; i++)
		printf("%02x ", static_cast<const uint8_t*>(data)[i]);
	printf("\n");
#endif

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

int TransportUnix::read(void *data, uint32_t count) {
	uint32_t bytesRead = 0;

	// read data up to 'count' bytes but return when we have not
	// received anything in 0.5s
	uint8_t *toRead = (uint8_t*) data;
	do {
//		waitForData(1, 500*1000); // TODO dont use for motorbus
		int r = ::recv(sock, toRead + bytesRead, count - bytesRead, MSG_DONTWAIT);
		if (r == -1) {
			if (errno == EBADF || errno == ECONNRESET) {
				close();
			}
			// TODO: improve error handling
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

bool TransportUnix::waitForData(uint32_t, Microsecond timeout) {
	struct timeval tvTimeout = { 0, (__suseconds_t)timeout.value() };
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);

	int s = select(sock+1, &readfds, 0, 0, &tvTimeout);
	if (s <= 0) {
		if (errno == EBADF || errno == ECONNRESET)
			this->close();

		return false; // timeout (or error)
	} else
		return true;  // data available
}
