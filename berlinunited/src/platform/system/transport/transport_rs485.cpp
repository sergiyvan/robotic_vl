#include "transport_rs485.h"

#include "debug.h"
#include "services.h"
#include "management/config/config.h"
#include "platform/system/timer.h"
#include "communication/comm.h"


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

//#if defined ROBOT2012 and not defined DESKTOP
//#include <linux/rs485.h>
//#include <asm-generic/ioctls.h>
//#endif


// For serial communication over RS485 line, RTS is used to switch between sending and
// receiving. This can either be done within this code, or by using a kernel patch.
// #define HANDLE_RTS_MANUALLY // handle RTS manually

/*------------------------------------------------------------------------------------------------*/

TransportSerial485::TransportSerial485(std::string _port, int newBaudrate)
	: fd(-1)
	, port(_port)
	, baudrate(newBaudrate)
{
}

TransportSerial485::~TransportSerial485() {
	close();
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** opens serial connection
 **
 ** @return FALSE if serial connection could not be established
 */

bool TransportSerial485::open() {
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
	struct termios2 options;
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

//#if defined ROBOT2012 and not defined DESKTOP
//	// set the port in RS485 mode
//	struct serial_rs485_settings ctrl485;
//	ctrl485.flags = SER_RS485_MODE | SER_RS485_MODE_RTS | SER_RS485_RTS_TX_LOW;
//	ctrl485.delay_before_send = 0;
//	ctrl485.delay_after_send = 0;
//	int status = ioctl(fd, TIOCSRS485, &ctrl485);
//	if (status) {
//		ERROR("Unable to configure port in 485 mode, status (%i)\n", status);
//		return -1;
//	}
//#endif

#ifdef HANDLE_RTS_MANUALLY
	// set rts line to signal that we are ready to receive
	setRTS(true);
#endif

//	INFO("Serial transport %s opened successfully (baudrate: %d)", port.c_str(), baudrate);
	return true;
}

/// return whether connection was established
bool TransportSerial485::isConnected() {
	return fd != -1;
}

/// close connection
void TransportSerial485::close() {
	if (fd != -1)
		::close(fd);

	fd = -1;
}

/**
 ** Sets (or unsets) the RTS control line
 **
 ** @param  on   true to set RTS, false to unset
 */

void TransportSerial485::setRTS(bool on) {
#ifdef HANDLE_RTS_MANUALLY
	int controlBits;

	ioctl(fd, TIOCMGET, &controlBits);
	if (on)
		controlBits |= TIOCM_RTS;
	else
		controlBits &= ~TIOCM_RTS;

	ioctl(fd, TIOCMSET, &controlBits);
#endif
}

// wait until output is flushed
// References:
//   http://osdir.com/ml/linux.serial/2005-05/msg00001.html
//   http://osdir.com/ml/linux.serial/2005-05/msg00014.html
//   http://osdir.com/ml/linux.serial/2005-05/msg00015.html

void TransportSerial485::drain(int count) {
	unsigned int lsr;

	// time it takes to send count bytes in microseconds
	int duration = count * 10 * 1000000 / baudrate;
	usleep(duration);

	robottime_t start = getCurrentTime();

	do {
		ioctl(fd, TIOCSERGETLSR, &lsr);
	} while ( ! (lsr & TIOCSER_TEMT) && start + 10*milliseconds > getCurrentTime());

//	ioctl(fd, TCSBRK, 1); // same as tcdrain(fd)
}


/// write
int TransportSerial485::write(const void *data, uint32_t count) {
	if (fd == -1)
		return 0;

	const Microsecond SerialWriteTimeout = 100*microseconds;
	Microsecond    startTime          = getCurrentMicroTime();
	uint32_t       bytesWritten       = 0;
	const uint8_t *toSend = (const uint8_t*) data;

#ifdef HANDLE_RTS_MANUALLY
	// drop rts line to signal that we want to write
	setRTS(false);
#endif

	do {
		ssize_t w = ::write(fd, toSend + bytesWritten, count - bytesWritten);

		if (w == -1) {
			// TODO: improve error handling
		} else {
			bytesWritten += w;
		}
	} while (bytesWritten < count  &&  getCurrentMicroTime() - startTime < SerialWriteTimeout);

	// wait until OS buffer is empty
	drain(count);

#ifdef HANDLE_RTS_MANUALLY
	// set rts line to signal that we are ready to receive again
	setRTS(true);
#endif

	return bytesWritten;
}

/// read
int TransportSerial485::read(void *data, uint32_t count) {
	if (fd == -1)
		return 0;

	return ::read(fd, data, count);
}

/// wait for data
bool TransportSerial485::waitForData(uint32_t, Microsecond timeout) {
	if (fd == -1)
		return false;

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
