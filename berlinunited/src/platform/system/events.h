/** @defgroup The Event and Callback System
 ** @ingroup platform
 **
 ** Events is a simple event registration system where objects derived from
 ** EventCallback can register for an event and their eventCallback() function
 ** will be called whenever an event has been triggered.
 **
 ** This call will happen synchronously.
 **
 ** @note Do not mix up Events and Event (from thread.h)!
 **
 ** Events in this context are defined by their name. For easier processing,
 ** they are assigned an EventType (basically an integer) that should be used.
 **
 ** To register for an event and act upon you have to do the following:
 **
 **  -# your class needs to inherit from EventCallback and implement eventCallback().
 **    In eventCallback() you need to act depending on the EventType.
 **  -# your class needs to register for the required event, e.g.:
 **    services.getEvents().registerForEvent(EVT_IMAGE_PROCESSED, this);
 **    where EVT_IMAGE_PROCESSED is the EventType you are interested in.
 **    This is done e.g. in the constructor of your class
 **  -# when done, your class must unregister:
 **    services.getEvents().unregisterForEvent(EVT_IMAGE_PROCESSED, this);
 **    where EVT_IMAGE_PROCESSED is the EventType you want to unregister.
 **    This should be done as early as possible, latest at the beginning(!) of
 **    the destructor of your class.
 **
 ** To trigger an event, you do not need to register for it, but simply
 ** call services.getEvents().trigger(EVT_IMAGE_PROCESSED, params) with
 ** EVT_IMAGE_PROCESSED is the EventType you want to trigger, and params
 ** is a void* with event specific data. Remember that a call to trigger
 ** will execute synchronously, i.e. the trigger() call will only return
 ** once ALL callbacks that registered for that particular event have finished
 ** their execution.
 **
 ** To define a new event, you need to declare, define and finally register
 ** it. The declaration must be done in a header (as the event variable holding
 ** the EventType needs to be seen by the other files that use this event) by
 ** using the DECLARE_EVENT_ID macro. It's only parameter is the id (name) of
 ** the macro which should start with EVT_ and be all upper-case.
 **
 ** In a source file, you then need to define the EventType by using the
 ** DEFINE_EVENT_ID macro, giving it the same id (which will also be the name
 ** of the variable holding the EventType) as well as a short description.
 **
 ** Finally you need to make the new event known to the Events manager and
 ** actually get assigned the EventType number by using the REGISTER_EVENT_ID
 ** macro in e.g. your class's constructor.
 **
 ** @{
 */

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include "thread.h"

#include <map>
#include <vector>


/*------------------------------------------------------------------------------------------------*/

/**
 ** Macros to provide events
 */

/// declare the event type for the id
#define DECLARE_EVENT_ID(id) \
		extern EventType id;

/// define the event type for the id
#define DEFINE_EVENT_ID(id, description) \
		EventType id = EVT_INVALID; \
		static const std::string EventTypeDescription##id(description);

/// actually register the event and get an EventType assigned
#define REGISTER_EVENT_ID(id) \
		{ \
			static CriticalSection cs(#id); \
			CriticalSectionLock lock(cs); \
			if (id == EVT_INVALID) \
				id = services.getEvents().registerEventType(#id, EventTypeDescription##id); \
		}

/// actually register the event and get an EventType assigned (this variant
/// is for ServicesBase as it can not use the services variable)
#define REGISTER_BASE_EVENT_ID(id) \
		{ \
			static CriticalSection cs(#id); \
			CriticalSectionLock lock(cs); \
			if (id == EVT_INVALID) \
				id = getEvents().registerEventType(#id, EventTypeDescription##id); \
		}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Events that can be registered for / triggered.
 */

typedef enum {
	  EVT_INVALID                         /// dummy/placeholder event, do not use
	, EVT_FIRST_USER_DEFINED_EVENT
} EventType;


/*------------------------------------------------------------------------------------------------*/


/**
 ** In order to be notified (via callback) of triggered events, a class must
 ** inherit from the EventCallback class (think 'Interface') and also register
 ** itself with the Events class.
 */

class EventCallback {
public:
	virtual ~EventCallback() {}

	/** The eventCallback function is called when a registered event has
	 ** been triggered.
	 **
	 ** @param eventType    EventType that was triggered
	 ** @param data         Additional event specific data
	 */
	virtual void eventCallback(EventType eventType, void* data) = 0;
};


/*------------------------------------------------------------------------------------------------*/

/**
 ** Events is responsible for managing the triggering and notification of events.
 ** Classes derived from EventCallback can register for a certain event with the
 ** Events instance and will receive a callback when the event is triggered.
 */

class Events {
public:
	Events();
	virtual ~Events();

	bool registerForEvent(EventType eventTypeToRegister, EventCallback *callback);
	bool unregisterForEvent(EventType eventTypeToUnregister, EventCallback *callback);

	bool trigger(EventType eventTypeToTrigger, void* param=0);

	EventType registerEventType(const std::string &name, const std::string &description);
	EventType getEventType(const std::string &name);

protected:
	/// vector of EventCallback pointers
	typedef std::vector<EventCallback*> EventVector;

	/// map assigned EventTypes to EventCallback pointers
	std::map<EventType, EventVector> registry;

	/// map assigning EventNames to EventTypes
	std::map<std::string, EventType> typeRegistry;

private:
	CriticalSection cs;
};

/**
 ** @}
 */
#endif
