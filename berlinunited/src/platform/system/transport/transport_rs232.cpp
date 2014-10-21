#include "transport_rs232.h"
#include "platform/system/timer.h"
#include "communication/comm.h"
#include "services.h"
#include "debug.h"
#include "management/config/config.h"


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


//#define RS232DEBUG


/*------------------------------------------------------------------------------------------------*/

TransportSerial232::TransportSerial232(std::string _port, int newBaudrate)
	: fd(-1)
	, port(_port)
	, baudrate(newBaudrate)
{
}

TransportSerial232::~TransportSerial232() {
	close();
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** opens serial connection
 **
 ** @return FALSE if serial connection could not be established
 */

bool TransportSerial232::open() {
	fd = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		ERROR("Could not open serial port %s", port.c_str());
		return false;
	}

	// when reading, return immediately
	fcntl(fd, F_SETFL, FNDELAY);

	// set line (do this before setting the termios structure, as our kernel patch
	// for serial_pxa_set_termios is acting according to serial.flags!

	struct serial_struct serial;
	ioctl(fd, TIOCGSERIAL, &serial);

	int lowLatency = services.getConfig().get<bool>("SERIALLOWLATENCY", 1);
	if (lowLatency != 0)
		serial.flags |= ASYNC_LOW_LATENCY;  /* enable low latency  */
	else {
		INFO("disabling low latency\n");
		serial.flags &= ~ASYNC_LOW_LATENCY; /* disable low latency */
	}
	ioctl(fd, TIOCSSERIAL, &serial);

	// get currently active settings
	struct termios2 options = { 0 };
	ioctl(fd, TCGETS2, &options);

	// local line that supports reading
	options.c_cflag |= (CLOCAL | CREAD);

	// set 8N1
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// set baudrate
	options.c_ospeed = baudrate;
	options.c_ispeed = baudrate;
	options.c_cflag  &= ~CBAUD;
	options.c_cflag  |= BOTHER;

	// disable hardware flow control
	options.c_cflag &= ~CRTSCTS;

	// use raw mode (see "man cfmakeraw")
	options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	options.c_cflag &= ~(CSIZE | PARENB);
	options.c_cflag |= CS8;

	ioctl(fd, TCSETS2, &options);

//	INFO("Serial RS232 transport %s opened successfully (baudrate: %d)", port.c_str(), baudrate);
	return true;
}

/// return whether connection was established
bool TransportSerial232::isConnected() {
	return fd != -1;
}

/// close connection
void TransportSerial232::close() {
	if (fd != -1)
		::close(fd);

	fd = -1;
}


/// write
int TransportSerial232::write(const void *data, uint32_t count) {
	// before writing, make sure the send queue is empty
//	unsigned int lsr;
//	do {
//		ioctl(fd, TIOCSERGETLSR, &lsr);
//		printf("LSR %x\n", lsr);
//	} while ( ! (lsr & TIOCSER_TEMT) );

	const Microsecond SerialWriteTimeout = 100*microseconds;
	Microsecond    startTime          = getCurrentMicroTime();
	uint32_t       bytesWritten       = 0;
	const uint8_t *toSend = (const uint8_t*) data;

	do {
		ssize_t w = ::write(fd, toSend + bytesWritten, count - bytesWritten);

		if (w == -1) {
			// TODO: improve error handling
		} else {
			if (bytesWritten == 0)
				startTime = getCurrentMicroTime();
			bytesWritten += w;
		}
	} while (bytesWritten < count  &&  getCurrentMicroTime() - startTime < SerialWriteTimeout);

	if (bytesWritten != count)
		ERROR("Insufficient write on serial port");
#ifdef RS232DEBUG
	printf("written %d bytes: ", bytesWritten);
	for (unsigned int i=0; i < bytesWritten; i++)
		printf("%02x ", toSend[i]);
	printf("\n");
#endif

	// We want to wait for the data to have left the system. This is
	// only effectively possible using polling. To save CPU time, we
	// do some rough calculation on how long it should take and just
	// take a quick nap.
	int32_t uDelay = (1000000 /* us */ * (bytesWritten * 10 /* 8 bits/byte + start/stop bit */)) / baudrate
	                 - (getCurrentMicroTime() - startTime).value();
	// uDelay -= ???; --- remove some, as we always have overhead?
	if (uDelay > 0) {
		usleep(uDelay);
	}

	// now poll until the fifo is finished transmitting
	unsigned int lsr = 0;
	robottime_t start = getCurrentTime();

	do {
		if (-1 == ioctl(fd, TIOCSERGETLSR, &lsr)) {
			break;
		}
	} while ( ! (lsr & TIOCSER_TEMT) && start + 10*milliseconds > getCurrentTime());

	return bytesWritten;
}

/// read
int TransportSerial232::read(void *data, uint32_t count) {
	int bytesRead = ::read(fd, data, count);

#ifdef RS232DEBUG
	if (bytesRead > 0) {
		printf("read %d bytes: ", bytesRead);
		for (int i=0; i < bytesRead; i++)
			printf("%02x ", static_cast<uint8_t*>(data)[i]);
		printf("\n");
	}
#endif

	return bytesRead;
}

/// wait for data
bool TransportSerial232::waitForData(uint32_t, Microsecond timeout) {
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
