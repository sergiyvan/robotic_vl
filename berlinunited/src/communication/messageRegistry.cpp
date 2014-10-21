#include "messageRegistry.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>


/*------------------------------------------------------------------------------------------------*/

/** Constructor
 */

MessageRegistry::MessageRegistry()
	: cs()
	, registry()
{
	cs.setName("MessageRegistry");
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor
 */

MessageRegistry::~MessageRegistry() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Register a callback for a proto message with the namespace 'de.fumanoids.message'.
 **
 ** @param callback   MessageCallback object to call back when message is received
 ** @param msgName    Name of message (case sensitive). This is the extension Name
 **                   in the proto files.
 **
 ** @return true iff registration succeeded
 */
bool MessageRegistry::registerMessageCallback(MessageCallback* callback, const std::string &msgName) {
	std::string fullMsgName = "de.fumanoids.message." + msgName;
	MessageRegistration msgReg(fullMsgName, callback);

	if (registry.find(fullMsgName) != registry.end()) {
		registry[fullMsgName].push_back(msgReg);
	} else {
		// message name is unknown
		std::vector<MessageRegistration> v;
		v.push_back(msgReg);
		registry[fullMsgName] = v;
	}

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Unregister a callback for a proto message
 **
 ** @param callback        MessageCallback object that was registered to be called back
 ** @param msgName         Name of message (case sensitive)
 **
 ** @return true iff unregistration succeeded
**/

bool MessageRegistry::unregisterMessageCallback(MessageCallback* callback, const std::string &msgName) {
	CriticalSectionLock lock(cs);

	Registry::iterator it;
	std::string fullMsgName;
	fullMsgName = "de.fumanoids.message." + msgName;

	// find entry
	it = registry.find(fullMsgName);
	if (it == registry.end()) {
		return false;
	} else {

		for (std::vector<MessageRegistration>::iterator it_msg = it->second.begin();
		     it_msg != it->second.end();
		     ++it_msg)
		{
			if (callback == (*it_msg).callback) {
				(*it).second.erase(it_msg);

				if ((int) (*it).second.size() < 1)
					registry.erase(it);

				return true;
			}
		}

		// callback did not exist
		return false;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Handle a message and delegate calls to the callbacks that are affected.
 **
 ** @param msg         Message to handle
 ** @param senderID    ID of sender
 ** @param remote      Remote connection
 */

void MessageRegistry::handleMessage(const google::protobuf::Message &msg, int32_t senderID, RemoteConnectionPtr remote) {
	// extract all populated fields from the message
	std::vector<const google::protobuf::FieldDescriptor*> fields;
	msg.GetReflection()->ListFields(msg, &fields);

	// check each field for something that we know of
	for (uint32_t i = 0; i < fields.size(); i++) {
		Registry::iterator it = registry.find(fields[i]->full_name());

		if (it != registry.end()) {

			const google::protobuf::Message &msg_data =
					msg.GetReflection()->GetMessage(msg, fields[i]);

			for (std::vector<MessageRegistration>::iterator it_msg = it->second.begin();
			     it_msg != it->second.end(); ++it_msg)
			{
				if ((*it_msg).callback->messageCallback(it->first, msg_data, senderID, remote)) {
					break;
				}
			}
		}
	}
}
