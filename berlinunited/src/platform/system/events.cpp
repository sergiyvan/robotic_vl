#include "events.h"

#include "debug.h"

/*------------------------------------------------------------------------------------------------*/

/** Constructor
 */

Events::Events()
	: registry()
	, typeRegistry()
	, cs()
{
	cs.setName("Events");
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor
 */

Events::~Events() {
}


/*------------------------------------------------------------------------------------------------*/

/** Register a callback for an event.
 **
 ** @param eventTypeToRegister  For which EventType to register
 ** @param callback             EventCallback to use when the event is triggered
 **
 ** @return true iff callback was successfully registered for this event
 */

bool Events::registerForEvent(EventType eventTypeToRegister, EventCallback *callback) {
	CriticalSectionLock lock(cs);

	if (eventTypeToRegister == EVT_INVALID) {
		ERROR("Attempt to register for invalid event.");
		ASSERT(false);
		return false;
	}
	EventVector &eventVector = registry[eventTypeToRegister];
	eventVector.push_back(callback);
	return false;
}


/*------------------------------------------------------------------------------------------------*/

/** Unregister a callback.
 **
 ** if function trigger() is beeing called and not finished,
 ** it might call this callback even though it's unregistered
 **
 ** @param eventTypeToUnregister  Which EventType to unregister
 ** @param callback               EventCallback that is to be unregistered
 **
 ** @return true iff callback was unregistered, false in case of error (e.g. not registered)
 */

bool Events::unregisterForEvent(EventType eventTypeToUnregister, EventCallback *callback) {
	CriticalSectionLock lock(cs);

	EventVector &eventVector = registry[eventTypeToUnregister];
	EventVector::iterator it, end = eventVector.end();
	for (it = eventVector.begin(); it != end; it++) {
		if (*it == callback) {
			eventVector.erase(it);
			return true;
		}
	}

	return false;
}


/*------------------------------------------------------------------------------------------------*/

/** Trigger an event, this happens synchronously.
 **
 ** @param eventTypeToTrigger  Which event should be triggered
 ** @param param               Optional parameter, context based on the EventType
 **
 ** @return true iff event was triggered
 */

bool Events::trigger(EventType eventTypeToTrigger, void* param) {
	EventVector eventVector;

	// Create save copy of registry[eventTypeToTrigger]
	// like this triggered events can trigger again, while
	// also being allowed to register or unregister callbacks asynchron
	{
		CriticalSectionLock lock(cs);
		eventVector = registry[eventTypeToTrigger];
	}
	EventVector::iterator it, end = eventVector.end();
	for (it = eventVector.begin(); it != end; it++) {
		(*it)->eventCallback(eventTypeToTrigger, param);
	}

	return false;
}


/*------------------------------------------------------------------------------------------------*/

/** Register a new event.
 **
 ** @param name          Textual ID (name) of the event
 ** @param description   Description when event is triggered
 **
 ** @return assigned event type ID to be used, EVT_INVALID on error
 */

EventType Events::registerEventType(
		  const std::string &name
		, const std::string &description)
{
	CriticalSectionLock lock(cs);

	if (typeRegistry.find(name) != typeRegistry.end()) {
		printf("ERROR: Event name %s is already registered.\n", name.c_str());
		return EVT_INVALID;
	}

	EventType newEventType = (EventType)(EVT_FIRST_USER_DEFINED_EVENT);
	for (std::map<std::string, EventType>::iterator it = typeRegistry.begin();
	     it != typeRegistry.end();
	     it++)
	{
		if ((int)it->second >= (int)newEventType)
			newEventType = (EventType)(it->second + 1);
	}

	typeRegistry[name] = newEventType;
	return newEventType;
}


/*------------------------------------------------------------------------------------------------*/

/** Retrieve event type for an event.
 **
 ** @param name       Textual ID (name) of the event
 **
 ** @return event type ID assigned, EVT_INVALID if event name is unknown
 */

EventType Events::getEventType(const std::string &name) {
	CriticalSectionLock lock(cs);

	if (typeRegistry.find(name) != typeRegistry.end()) {
		return typeRegistry[name];
	} else
		return EVT_INVALID;
}
