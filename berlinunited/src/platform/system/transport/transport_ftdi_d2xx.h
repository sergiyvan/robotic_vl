#ifndef __FTDID2XXTRANSPORT_H__
#define __FTDID2XXTRANSPORT_H__

#include "transport.h"
#include <inttypes.h>
#include <ftdi.h>

#include "ftd2xx.h"

/*------------------------------------------------------------------------------------------------*/

/**
 **
 ** @ingroup transport
 **
 */

class TransportUSBFTDID2XX : public Transport {
protected:
	FT_HANDLE ftHandle;

public:
	TransportUSBFTDID2XX(int baudrate=1000000);
	virtual ~TransportUSBFTDID2XX() {}

	/// open connection
	virtual bool open();

	/// returns whether connection to bus was established
	virtual bool isConnected();

	/// close connection
	virtual void close();

	/// write
	virtual int write(const void *data, uint8_t count);

	/// read
	virtual int read(void *data, uint8_t count);

	/// wait for data
	virtual bool waitForData(uint32_t, Microsecond timeout);
};

#endif // #ifndef
