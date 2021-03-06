/** @defgroup The Message Registry System
 **
 ** In order to keep the global protobuf message definition and handling code clean
 ** of any specific knowledge what it contains, sub-messages and their handling must
 ** be defined outside these files (for example outside FUmanoid.proto and the Comm class).
 **
 ** In order to achieve this goal, we make use of protobuf extensions. If you want to
 ** add a new message to be sent or received, create your own protobuf message and add
 ** it by means of an extension to the FUmanoid 'Message'.
 **
 ** @code
extend Message {
	optional MyNewMessage myNewMessage = <ID>;
}
 ** @endcode
 **
 ** You will need to set <ID> to a unique ID.
 **
 ** @TODO: define how IDs are managed
 **
 ** Sending a message of this type out can be done as usual.
 **
 ** Receiving a message of this type requires that you register the name of the field
 ** used in the extension with the MessageRegistry class and provide a reference to a
 ** MessageCallback. If a message 'Message' with 'myNewMessage' is received, your
 ** callback function will be called.
 **
 ** @{
 */


#ifndef MESSAGEREGISTRY_H_
#define MESSAGEREGISTRY_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <map>
#include <string>

#include "communication/remoteConnection.h"
#include "platform/system/thread.h"
#include "utils/patterns/singleton.h"


/*------------------------------------------------------------------------------------------------*/

/** Classes that want to receive callbacks for received messages need to inherit from this abstract
 ** base class.
 */
class MessageCallback {
public:
	virtual ~MessageCallback() {}

	/** Method used for callback.
	 **
	 ** @param messageName   Name of message received (matches one of previously registered ones)
	 ** @param msg           Message (needs to be cast to correct type)
	 ** @param senderID      ID of sending robot
	 ** @param remote        Remote connection. This is a unique pointer, use std::move to take possession of it.
	 **
	 ** @return true if message was handled (and remote got possibly invalidated), false otherwise (and remote has not been invalidated)
	 */
	virtual bool messageCallback(
	        const std::string               &messageName,
	        const google::protobuf::Message &msg,
	        int32_t                          senderID,
	        RemoteConnectionPtr             &remote) = 0;
};


/*------------------------------------------------------------------------------------------------*/

/** Class holding information about a registered protobuf message and the
 * corresponding callback.
 */

class MessageRegistration {
public:
	MessageRegistration()
		: msgName("")
		, callback(0)
	{}

	MessageRegistration(
			std::string      _msgName,
			MessageCallback *_callback
	)
	: msgName(_msgName)
	, callback(_callback)
	{
	}

	// Overloaded copy constructor. Keep with the shallow copy as
	// we can not instantiate a new callback handler.
	MessageRegistration(const MessageRegistration& other)
		: msgName(other.msgName)
		, callback(other.callback)
	{
	}

	// Overloaded assignment operator. Keep with the shallow copy as
	// we can not instantiate a new callback handler.
	MessageRegistration& operator=(const MessageRegistration& other) {
		this->msgName  = other.msgName;
		this->callback = other.callback;
		return *this;
	}

	friend class MessageRegistry;

private:
	std::string      msgName;
	MessageCallback *callback;
};


/*------------------------------------------------------------------------------------------------*/

/** All proto messages and callback objects need to be registered with this class.
 */

class MessageRegistry {
public:
	MessageRegistry();
	virtual ~MessageRegistry();

	void registerMessage();
	void handleMessage(const google::protobuf::Message &msg, int32_t id, RemoteConnectionPtr remote);

	bool registerMessageCallback(MessageCallback* callback, const std::string &msgName);
	bool unregisterMessageCallback(MessageCallback* callback, const std::string &msgName);

protected:
	CriticalSection cs;

	typedef std::map<std::string, std::vector<MessageRegistration> > Registry;
	Registry registry;
};

/** @}
 */
#endif /* MESSAGEREGISTRY_H_ */
