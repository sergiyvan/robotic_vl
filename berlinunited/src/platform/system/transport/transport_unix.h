#ifndef __UNIXTRANSPORT_H__
#define __UNIXTRANSPORT_H__

#include "transport.h"
#include "platform/system/thread.h"
#include <sys/un.h>

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** The Unix transport implements communication via a Unix socket.
 **
 ** @ingroup transport
 **
 */

class TransportUnix : public Transport {
protected:
	/// file descriptor for the TCP port
	int sock;

	std::string location;

public:
	TransportUnix(const char *location);
	TransportUnix(int sock, const char *location);
	virtual ~TransportUnix();

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
