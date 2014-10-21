#ifndef __FTDITRANSPORT_H__
#define __FTDITRANSPORT_H__

#include "transport.h"
#include <inttypes.h>
#include <ftdi.h>

/*------------------------------------------------------------------------------------------------*/

/**
 ** The FTDI transport implements communication via a FTDI (USB2Serial) chip.
 **
 ** @ingroup transport
 **
 */

class TransportUSBFTDI : public Transport {
protected:
	/// file descriptor for the serial port
	struct ftdi_context ftdic;

	/// set to true if we are connected
	bool connected;

	std::string port;
	int baudrate;

public:
	TransportUSBFTDI(const std::string port, int baudrate); // : connected(false) {}
	virtual ~TransportUSBFTDI() {}

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

};

#endif
