#ifndef __SERIALTRANSPORT485_H__
#define __SERIALTRANSPORT485_H__

#include "transport.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The serial transport implements communication via RS485 on a standard
 ** serial port. This requires that the TX and RX lines are connected to the
 ** RS485 data line(s) via a MAX485 chip. The RTS line is used to switch
 ** between sending and receiving data.
 **
 ** @ingroup transport
 **
 */

class TransportSerial485 : public Transport {
protected:
	/// file descriptor for the serial port
	int fd;

	/// device file for serial port
	std::string port;

	/// baudrate in use
	int baudrate;

	void setRTS(bool on);

	void drain(int byteCount);

public:
	TransportSerial485(const std::string port, int baudrate);
	virtual ~TransportSerial485();

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
