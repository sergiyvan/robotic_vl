#ifndef UDPHANDLER_H_
#define UDPHANDLER_H_

#include "commHandler.h"

// forward declaration
class RemoteConnection;
class TransportUDP;


/** The UDP Comm Handler is responsible for managing incoming UDP requests.
 **
 */
class UDPHandler : public CommHandler {
public:
	UDPHandler(CommHandlerManager *manager);
	virtual ~UDPHandler();

	/// name of thread
	virtual const char* getName() const override {
		return "UDPHandler";
	}

	virtual void init(int port, int broadcastPort=0) override;
	virtual void threadMain() override;

	RemoteConnection *getBroadcastConnection();

protected:

	CriticalSection     cs;
	TransportUDP       *transport;

	// we are using pointers, it does not make sense to copy this handler so prevent it
	UDPHandler(const UDPHandler &) = delete;
	UDPHandler& operator=(const UDPHandler &) = delete;
};

#endif /* UDPHANDLER_H_ */
