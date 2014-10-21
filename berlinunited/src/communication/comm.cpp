#include "communication/comm.h"

#include "services.h"

#include "remoteConnection.h"
#include "commHandler.h"
#include "tcpHandler.h"
#include "udpHandler.h"

#include "platform/system/timer.h"
#include "management/config/configRegistry.h"
#include "management/config/config.h"
#include "debug.h"
#include "messageRegistry.h"

#include "msg_segment.pb.h"

#include <google/protobuf/io/gzip_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include <sys/time.h>

#include <string>
#include <sstream>


/*------------------------------------------------------------------------------------------------*/

// The default port number that the server should listen to and send packets to
// other robots/PC. On desktop builds we will use a different local port number,
// in order to be able to use FUremote locally.
#if not defined DESKTOP
	#define DEFAULTCOMMPORT   11011
	#define DEFAULTREMOTEPORT DEFAULTCOMMPORT
#else
	#define DEFAULTCOMMPORT   11012
	#define DEFAULTREMOTEPORT 11011
#endif

#define COMM_HEADER_SIZE 4
#define OP_MESSAGE       42

#define MSG_FLAG_COMPRESSED 2


/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgPort       = ConfigRegistry::registerOption<uint16_t>   ("comm.port",           DEFAULTCOMMPORT,   "Port for communication (default 11011)");
	auto cfgRemoteIP   = ConfigRegistry::registerOption<std::string>("comm.remote.ip",      "",                "Remote IP to be used instead of broadcast packages");
	auto cfgRemotePort = ConfigRegistry::registerOption<uint16_t>   ("comm.remote.port",    DEFAULTREMOTEPORT, "Remote port for broadcast");
	auto cfgSimIP      = ConfigRegistry::registerOption<std::string>("comm.simulator.ip",   "",                "Secondary remote IP to be used instead of broadcast packages (e.g. simulator)");
	auto cfgSimPort    = ConfigRegistry::registerOption<uint16_t>   ("comm.simulator.port", 0,                 "Secondary remote port for broadcast (e.g. simulator)");

	auto cfgCompression          = ConfigRegistry::registerOption<bool>("comm.compression.enabled",  false,    "Whether to compress outgoing messages (1) or not (0)");
	auto cfgCompressionThreshold = ConfigRegistry::registerOption<int>("comm.compression.threshold", 32000,    "Minimum size (in bytes) of messages to compress");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
**/

Comm::Comm()
	: cs()
	, lastTimeCheck(0)
	, totalBytesSent(0)
	, totalBytesRead(0)
	, compressionThreshold(UINT_MAX)
	, handlers()
	, broadcastConnection(0)
	, simulatorBroadcastConnection(0)

{
	cs.setName("Comm::cs");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
**/

Comm::~Comm() {
	cancel();

	for (std::vector<CommHandler*>::iterator it = handlers.begin(); it != handlers.end(); ++it) {
		(*it)->cancel(true);
		delete *it;
	}

	handlers.clear();
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Initialize Comm and all required communication objects.
 **
 ** @return true iff initialization succeeded
 */

bool Comm::init() {
	CriticalSectionLock lock(cs);

	// get compression info
	if (false == cfgCompression->get())
		compressionThreshold = UINT_MAX;
	else
		compressionThreshold = cfgCompressionThreshold->get();

	// get ports to use
	int simBroadcastPort = cfgSimPort->get();
	int localPort        = cfgPort->get();
	int remotePort;

	// remote port should match the comm port ONLY if a comm port is specified
	if (cfgPort->isSet()) {
		remotePort = cfgRemotePort->get(localPort);
	} else
		remotePort = cfgRemotePort->get();


	// create UDP handler (usually connected to FUremote and team broadcast)
	UDPHandler *udp = new UDPHandler(this);
	handlers.push_back(udp);
	udp->init(localPort, remotePort);
	broadcastConnection.reset(udp->getBroadcastConnection());
	udp->run();

	// create second UDP handler if we are using the simulator as it will need
	// a communication channel for certain data exchange (TODO: for now) but also
	// in order to distribute UDP packages between the robots
	if (simBroadcastPort > 0) {
		UDPHandler *simUDP = new UDPHandler(this);
		handlers.push_back(simUDP);
		simUDP->init(0, simBroadcastPort);
		simulatorBroadcastConnection.reset(simUDP->getBroadcastConnection());
		simUDP->run();
	}

	// for larger data also offer a TCP channel (incoming requests only)
	TCPHandler *tcp = new TCPHandler(this);
	handlers.push_back(tcp);
	tcp->init(localPort);
	tcp->run();

	// start comm thread
	run();

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Thread main function
 **
**/

void Comm::threadMain() {
	while (isRunning()) {
		sleep(1);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
**/

bool Comm::process(RemoteConnectionPtr remote) {
	// read request and handle it
	uint16_t messageType;
	uint16_t messageDummy;
	uint32_t messageSize;

	/* the header of each message is defined as
	 *   uint16_t operationID (only valid: OP_MESSAGE)
	 *   uint8_t  flags
	 *   uint8_t  unused // set to 0
	 */

	int32_t received = remote->read((uint8_t*)&messageType, 2);
	if (received != sizeof messageType) {
		return false;
	}

	// we only handle message type OP_MESSAGE (protobuf)
	messageType = ntohs(messageType);
	if (messageType != OP_MESSAGE)
		return false;

	// read flags and the unused byte
	received = remote->read((uint8_t*)&messageDummy, 2);
	if (received != 2)
		return false;

	/* now we need to have a quick compatibility fix for old
	 * software versions, where message 42 was preceeded not
	 * by
	 *
	 *   0-3 message type (big endian) (see below)
	 *   4-7 length of message in bytes (big endian)
	 *
	 * whereas the new version just has the length. So we check
	 * whether type is zero - if it is, we assume it's the old
	 * format as hopefully nobody sends us an empty message :)
	 */

	received = remote->read((uint8_t*)&messageSize, 4);
	if (messageSize == 0) {
		received = remote->read((uint8_t*)&messageSize, 4);
	}

	if (received != 4)
		return false;

	messageSize = ntohl(messageSize);

	uint8_t *messageData = new uint8_t[messageSize];
	received = remote->read(messageData, messageSize);
	if (received != (signed)messageSize)
		return false;

	de::fumanoids::message::Message msg;
	msg.ParseFromArray(messageData, messageSize);
	delete[] messageData;

	if (msg.IsInitialized() == false) {
		WARNING("Received incorrect message: %s", msg.InitializationErrorString().c_str());
		return false;
	}

	handleMessage(msg, std::move(remote));
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Broadcast a protobuf message
 **
 ** @param  message      Protobuf message
**/

void Comm::broadcastMessage(de::fumanoids::message::Message &message) {
	if (simulatorBroadcastConnection && simulatorBroadcastConnection->isConnected())
		sendMessage(message, simulatorBroadcastConnection);

	if (broadcastConnection && broadcastConnection->isConnected())
		sendMessage(message, broadcastConnection);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Sends a protobuf message
 **
 ** @param  message      Protobuf message
 ** @param  connection   Recipient
**/

void Comm::sendMessage(de::fumanoids::message::Message &message, std::shared_ptr<RemoteConnection> connection) {
	sendMessage(message, connection.get());
}

void Comm::sendMessage(de::fumanoids::message::Message &message, RemoteConnection* connection) {
	if (nullptr == connection || false == connection->isConnected()) {
		//printf("connection not established, not sending\n");
		return;
	}

	// include our id
	message.set_robotid( services.getID() );

	if (message.IsInitialized() == false) {
		WARNING("Message sending failed due to wrong initialization: %s", message.InitializationErrorString().c_str());
		return;
	}

	uint32_t dataLen = (uint32_t)message.ByteSize();
	if (dataLen > compressionThreshold) {
		sendMessageCompressed(message, connection);
	} else {
		sendMessageUncompressed(message, connection);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool Comm::sendMessageUncompressed(de::fumanoids::message::Message &message, RemoteConnection* connection) {
	// include our id
	message.set_robotid( services.getID() );

	int dataLen = message.ByteSize();

	uint32_t  msgLen = COMM_HEADER_SIZE + 8 + dataLen;
	uint8_t  *msg    = (uint8_t*)malloc(msgLen);

	if (msg == 0) {
		ERROR("ERROR allocating memory for message structure");
		return false;
	}

	google::protobuf::io::ArrayOutputStream outputStream(msg + COMM_HEADER_SIZE + 8, dataLen);

	uint16_t operationBE = htons(OP_MESSAGE);
	uint8_t  flags       = 0;
	memset(msg, 0, COMM_HEADER_SIZE);
	memcpy(msg,   &operationBE, 2);
	memcpy(msg+2, &flags, 1);

	int dataLenBE = htonl(dataLen);
	memset(msg+4, 0,          4);
	memcpy(msg+8, &dataLenBE, 4);

	if (!message.SerializeToZeroCopyStream(&outputStream)) {
		ERROR("Could not serialize message");
		free(msg);
		return false;
	}

	bool success = false;
	if ((unsigned)msgLen > connection->getMaxPackageSize()) {
		success = sendMessageSegmented(msg, msgLen, connection);
	} else {
		success = connection->send(msg, msgLen);
	}

	free(msg);
	return success;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

bool Comm::sendMessageCompressed(de::fumanoids::message::Message &message, RemoteConnection* connection) {
	// include our id
	message.set_robotid( services.getID() );

	std::string compressedData;
	google::protobuf::io::StringOutputStream stream(&compressedData);

	google::protobuf::io::GzipOutputStream::Options gzipOptions;
	gzipOptions.format = google::protobuf::io::GzipOutputStream::GZIP;
	google::protobuf::io::GzipOutputStream gzipStream(&stream, gzipOptions);

	if (false == message.SerializeToZeroCopyStream( &gzipStream )) {
		printf("Could not serialize message\n");
		return false;
	}

	if (false == gzipStream.Close()) {
		printf("Error compressing message: %s\n", gzipStream.ZlibErrorMessage());
		return false;
	}

	// don't send data out if compressed size is larger than the uncompressed one
	if (message.ByteSize() < (int)compressedData.size()) {
		return sendMessageUncompressed(message, connection);
	}

	int dataLen = (int)compressedData.size();

	uint32_t  msgLen = COMM_HEADER_SIZE + 8 + dataLen;
	uint8_t  *msg    = (uint8_t*)malloc(msgLen);

	if (msg == 0) {
		ERROR("ERROR allocating memory for message structure");
		return false;
	}

	uint16_t operationBE = htons(OP_MESSAGE);
	uint8_t  flags       = MSG_FLAG_COMPRESSED;
	memset(msg, 0, COMM_HEADER_SIZE);
	memcpy(msg,   &operationBE, 2);
	memcpy(msg+2, &flags, 1);

	int dataLenBE = htonl(dataLen);
	memset(msg+4, 0,          4);
	memcpy(msg+8, &dataLenBE, 4);

	memcpy(msg + COMM_HEADER_SIZE + 8, compressedData.c_str(), dataLen);

	bool success;
	if ((unsigned)msgLen > connection->getMaxPackageSize()) {
		success = sendMessageSegmented(msg, msgLen, connection);
	} else {
		success = connection->send(msg, msgLen);
	}

	free(msg);
	return success;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

class UDPSegmentationCounter {
public:
	static uint64_t get() {
		CriticalSectionLock lock(cs);

		id++;
		return id;
	}

private:
	static CriticalSection cs;
	static uint64_t id;
};

uint64_t UDPSegmentationCounter::id = 0;
CriticalSection UDPSegmentationCounter::cs;


/*------------------------------------------------------------------------------------------------*/

/**
 ** Sends a segmented protobuf message.
 **
 ** @param msg Message to handle
 */

bool Comm::sendMessageSegmented(const uint8_t *data, uint32_t dataLen, RemoteConnection *connection) {
	// split message up
	int segmentSize = connection->getMaxPackageSize() - 128;

	// get unique ID for this session (FIXME: TODO: should be globally unique)
	uint64_t id = UDPSegmentationCounter::get();

	// get number of segments
	uint32_t count = dataLen / segmentSize + 1;

	// send each segment, abort if one segment fails
	bool success = true;
	for (uint32_t i = 0; i < count && success; i ++) {
		de::fumanoids::message::Message msg;
		de::fumanoids::message::MessageSegment &segment = *msg.MutableExtension(de::fumanoids::message::messageSegment);

		segment.set_id(id);
		segment.set_packagenumber(i);
		segment.set_totalpackagecount(count);

		segment.set_payload(data + i*segmentSize, i == count - 1 ? (dataLen - i*segmentSize) : segmentSize);

		success = success && sendMessageUncompressed(msg, connection);
		if (!success) {
			fprintf(stderr, "Segmented sending of %d bytes failed at segment %d of %d\n", dataLen, i, count);
		}
	}

	return success;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Evaluate incoming proto messages.
 **
 ** @param msg Message to handle
 */

void Comm::handleMessage(const de::fumanoids::message::Message &msg, RemoteConnectionPtr remote) {
	// execute callbacks
	services.getMessageRegistry().handleMessage(msg, msg.robotid(), std::move(remote));

	// from here on, 'remote' is INVALID!
	return;
}


/*------------------------------------------------------------------------------------------------*/

/** @deprecated
 **
 ** Outputs a log message about a received command, including sender IP and some excerpt
 ** from the data block.
 **
 ** @param commandName    Name of command
 ** @param data           Pointer to data block (may be NULL)
 ** @param dataLen        Length of data block
 ** @param remoteAddress  Address this command was sent from
 */
void Comm::logCommand(const char* commandName, uint8_t* data, uint16_t dataLen, struct sockaddr_in *remoteAddress) {
	std::string formattedData, sender;
	if (dataLen > 0) {
		formattedData = ": ";

		const int maxValues = 15;
		for (int i = 0; i < dataLen && i < maxValues; i++) {
			char tmp[16];

			if (isgraph(data[i]))
				sprintf(tmp, "%c ", data[i]);
			else
				sprintf(tmp, "%02x ", data[i]);

			formattedData += tmp;
		}

		if (dataLen > maxValues)
			formattedData += "...";
	}

	if (remoteAddress) {
		char tmp[32];
		uint32_t address = (uint32_t)remoteAddress->sin_addr.s_addr;
		uint8_t *addressP = (uint8_t*)&address;
		sprintf(tmp, "%d.%d.%d.%d", addressP[0], addressP[1], addressP[2], addressP[3]);
		sender = tmp;
	} else
		sender = "Unknown";

	INFO("Received command %s from %s with %d bytes of data %s", commandName, sender.c_str(), dataLen, formattedData.c_str());
}


/*------------------------------------------------------------------------------------------------*/

/** @deprecated
 **
 ** Process Time Sync command
 **
 ** @param data           Pointer to data block (may be NULL)
 ** @param dataLen        Length of data block
 ** @param remoteAddress  Address this command was sent from
 **
 ** @return true if command was successful
 */

bool Comm::processTimeSyncCommand(uint8_t *data, uint16_t dataLen, struct sockaddr_in *remoteAddress) {
	uint32_t time;
	memcpy(&time, data, 4);
	time = ntohl(time);
	printf("Got timesync command, time %d (0x%x) seconds \n", time, time);

	struct timeval tv = { (__time_t)time, 0 };
	settimeofday(&tv, 0);
	return true;
}
