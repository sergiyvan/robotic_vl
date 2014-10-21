#ifndef REMOTECONNECTION_H_
#define REMOTECONNECTION_H_


#include <inttypes.h>
#include <memory>


/*------------------------------------------------------------------------------------------------*/

/** A remote connection is an ABC object representing a connection to a remote party.
 **
 */
class RemoteConnection {
public:
	RemoteConnection();
	virtual ~RemoteConnection();

	virtual bool    isConnected()                               = 0;
	virtual bool    connect()                                   = 0;

	virtual int32_t read(uint8_t    *data, uint32_t dataLength) = 0;
	virtual bool    send(void const *data, uint32_t dataLength) = 0;

	virtual uint32_t getMaxPackageSize()                        = 0;
};


/*------------------------------------------------------------------------------------------------*/

typedef std::unique_ptr<RemoteConnection> RemoteConnectionPtr;

#endif /* REMOTECONNECTION_H_ */
