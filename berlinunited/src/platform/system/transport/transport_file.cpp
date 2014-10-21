#include "transport_file.h"
#include "platform/system/timer.h"
#include "debug.h"

#include <stdio.h>         // Standard input/output definitions
#include <string.h>        // String function definitions
#include <unistd.h>        // UNIX standard function definitions
#include <fcntl.h>         // File control definitions
#include <errno.h>         // Error number definitions
#include <inttypes.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <linux/types.h>
#include <linux/serial.h>


/*------------------------------------------------------------------------------------------------*/

TransportFile::TransportFile(std::string _filename)
	: fd(-1)
	, filename(_filename)
{
}

TransportFile::~TransportFile() {
	close();
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** opens file
 **
 ** @return FALSE if file could not be opened
 */

bool TransportFile::open() {
	fd = ::open(filename.c_str(), O_RDWR | O_CREAT, 00644);
	if (fd == -1) {
		ERROR("Could not open file %s", filename.c_str());
		return false;
	}

//	INFO("File %s opened successfully", filename.c_str());
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** opens file read-only
 **
 ** @return FALSE if file could not be opened
 */

bool TransportFile::openReadOnly() {
	fd = ::open(filename.c_str(), O_RDONLY, 00644);
	if (fd == -1) {
		ERROR("Could not open file %s", filename.c_str());
		return false;
	}

//	INFO("File %s opened successfully", filename.c_str());
	return true;
}

/// return whether connection was established
bool TransportFile::isConnected() {
	return fd != -1;
}

/// close connection
void TransportFile::close() {
	if (fd != -1)
		::close(fd);

	fd = -1;
}


/// write
int TransportFile::write(const void *data, uint32_t count) {
	const Microsecond writeTimeout = 100*microseconds;
	Microsecond    startTime      = getCurrentMicroTime();
	uint32_t       bytesWritten   = 0;
	const uint8_t *toSend = (const uint8_t*) data;
	do {
		ssize_t w = ::write(fd, toSend + bytesWritten, count - bytesWritten);

		if (w == -1) {
			// TODO: improve error handling
		} else {
			bytesWritten += w;
		}
	} while (bytesWritten < count  &&  getCurrentMicroTime() - startTime < writeTimeout);

/*
	printf("written %d bytes: ", bytesWritten);
	for (int i=0; i < bytesWritten; i++)
		printf("%02x ", data[i]);
	printf("\n");
*/
	return bytesWritten;
}

/// read
int TransportFile::read(void *data, uint32_t count) {
	int bytesRead = ::read(fd, data, count);
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

/// wait for data
bool TransportFile::waitForData(uint32_t, Microsecond timeout) {
	struct timeval tvTimeout = { 0, (__suseconds_t)timeout.value() };
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	int s = select(fd+1, &readfds, 0, 0, &tvTimeout);
	if (s <= 0)
		return false; // timeout (or error)
	else
		return true;  // data available
}
