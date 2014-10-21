#ifndef __TRANSPORTSERVER_H__
#define __TRANSPORTSERVER_H__

#include "transport.h"
#include "utils/units.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

/**
 ** A TransportServer is similar to a factory. It waits for incoming connections and creates an
 ** appropriate transport object.
 **
 ** @ingroup transport
 **
 */

class TransportServer {
public:
	virtual ~TransportServer() {}

	/// setup listening process
	virtual bool init() = 0;

	/// listen for connection
	virtual Transport* waitForIncoming(Microsecond timeout) = 0;

	/// clean up
	virtual void close() = 0;
};

#endif
