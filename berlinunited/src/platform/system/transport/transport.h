/**
 * @defgroup transport The Transport System
 * @ingroup platform
 *
 * Sending and receiving data via various communication channels (for example
 * serial lines, UDP sockets, TCP sockets, unix sockets, files, etc) is handled
 * by Transport classes that hide the specific details and allow for easy change
 * of channels.
 *
 * @{
 */

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <inttypes.h>

#include "utils/units.h"


/*------------------------------------------------------------------------------------------------*/

/**
 ** Transport is an abstract base class which implementations will offer different ways
 ** to send and receive data.
 */

class Transport {
  public:
	Transport() {}
	virtual ~Transport() {}

	/// open connection
	virtual bool open() = 0;

	/// checks whether connection was established
	virtual bool isConnected() = 0;

	/// close connection
	virtual void close() = 0;

	/// write
	virtual int write(const void *data, uint32_t count) = 0;

	/// read
	virtual int read(void *data, uint32_t count) = 0;

	/// wait until timeout expires or data is available, returns true if data available,
	/// false if timeout
	virtual bool waitForData(uint32_t bytesToWaitFor, Microsecond timeout) = 0;
};

/** @}
 */

#endif
