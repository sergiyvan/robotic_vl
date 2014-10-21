#ifndef __SERIALTRANSPORT232_H__
#define __SERIALTRANSPORT232_H__

#include "transport.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The serial transport implements communication via a standard Linux serial device.
 **
 ** @ingroup transport
 **
 */

class TransportSerial232 : public Transport {
protected:
	/// file descriptor for the serial port
	int fd;

	/// device file for serial port
	std::string port;

	/// baudrate in use
	int baudrate;

public:
	TransportSerial232(const std::string port, int baudrate);
	virtual ~TransportSerial232();

	/// open connection
	virtual bool open();

	/// returns whether connection to bus was established
	virtual bool isConnected();

	/// close connection
	virtual void close();

	/// write
	virtual int write(const void *data, uint32_t count);

	/// read
	virtual int read(void *data, uint32_t count);

	/// wait for data
	virtual bool waitForData(uint32_t, Microsecond timeout);
};

#endif
