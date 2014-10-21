#ifndef __COMM_H__
#define __COMM_H__

#include "platform/system/thread.h"

#include "commHandlerManager.h"

#include <arpa/inet.h>
#include <stdarg.h>

#include <vector>
#include <map>

// forward declarations
namespace de {
	namespace fumanoids {
		namespace message {
			class Message;
		}
	}
}

class RemoteConnection;
class CommHandler;


/*------------------------------------------------------------------------------------------------*/
/**
 * @defgroup comm Communication
 *
 * The Communication handles all incoming and outgoing communication.
 *
\include ../doc/comm.txt doc/comm.txt
 *
 * @{
 */


/*------------------------------------------------------------------------------------------------*/

/**
 ** Comm maintains the connection to server and acts as the remote control interface.
 **
 ** The server object is the heart of all that involves parties outside this particular robot,
 ** i.e. other robots, the server and any human trying to interact with the robot.
 **
 ** All messages (in- and outgoing) are constructed in the exact same way to allow for a transparent
 ** processing. As we use UDP, the message size is known and doesn't need to be encoded specially.
 **
 ** See doc/comm.txt for details on message format and parameters.
 **
 */

class Comm
	: public Thread
	, public CommHandlerManager
{
protected:
	/// critical section
	CriticalSection cs;

	/// time we last asked for the correct time
	robottime_t lastTimeCheck;

	/// amount of data send and read in current round
	uint32_t     totalBytesSent;
	uint32_t     totalBytesRead;

	// minimum number of message bytes to enable compression
	uint32_t     compressionThreshold;

	/// thread main function, waits for incoming packets
	void threadMain();

	void handleMessage(const de::fumanoids::message::Message &msg, RemoteConnectionPtr remote);
	bool sendMessageSegmented(const uint8_t *message, uint32_t messageLength, RemoteConnection *connection);
	bool sendMessageCompressed(de::fumanoids::message::Message &message, RemoteConnection* remote);
	bool sendMessageUncompressed(de::fumanoids::message::Message &message, RemoteConnection* remote);

	void logCommand(const char* commandName, uint8_t* data, uint16_t dataLen, struct sockaddr_in *remoteAddress);

	bool processInitCommand(const uint8_t *data, uint16_t dataLen, struct sockaddr_in *remoteAddress);
	bool processPlayMotionCommand(uint8_t *data, uint16_t dataLen, struct sockaddr_in *remoteAddress);
	bool processTimeSyncCommand(uint8_t *data, uint16_t dataLen, struct sockaddr_in *remoteAddress);

	std::vector<CommHandler*> handlers;

public:
	Comm();
	virtual ~Comm();

	std::shared_ptr<RemoteConnection> broadcastConnection;
	std::shared_ptr<RemoteConnection> simulatorBroadcastConnection;

	/// get a descriptive name of the thread
	virtual const char* getName() const override{
		return "Comm";
	}

	bool init();

	void sendMessage(de::fumanoids::message::Message &message, std::shared_ptr<RemoteConnection> remote);
	void sendMessage(de::fumanoids::message::Message &message, RemoteConnection* remote);
	void broadcastMessage(de::fumanoids::message::Message &message);

	virtual bool process(RemoteConnectionPtr remote);
};


/** @} */ // end of group comm

#endif
