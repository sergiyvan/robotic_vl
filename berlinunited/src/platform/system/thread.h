#ifndef __THREAD_H__
#define __THREAD_H__

/** Wrappers for threads, mutexes, event notification (condition variables) and
 ** critical sections.
 **
 ** Originally, all these wrappers were based on the pthread library and were
 ** meant to simplify the use. Additionally they provided certain error checking
 ** (e.g. some deadlock handling).
 **
 ** For cross-platform compatibility reasons, the wrappers are now based on the
 ** C++11 implementations and mostly exist now for compatibility with the older
 ** code-base. Still, they provide some basic functionality to manage the
 ** classes (e.g. naming the threads/mutexes/... in order for better debug output).
 */

#include <thread>
#include <mutex>
#include <condition_variable>

#include <string>
#include <iostream>
#include <map>

#include "timer.h"

#include "utils/units.h"
#include "utils/namedInstance.h"


/*------------------------------------------------------------------------------------------------*/

#define MUTEX_DEADLOCK_TERMINATE_TIMEOUT_SECONDS (20*seconds)

/**
 ** Wrapper for a mutex
 **
 ** A mutex represents a resource that can only be used by one entity at a time. To
 ** take possession of the resource, it must be "locked". Should the resource already
 ** be in use, this will wait for it to become available again. Once the resource is
 ** not needed anymore, it should be unlocked so another thread may make use of it.
 **
 ** A "resource" in this sense usually means a variable or code segment.
 **
 ** For further reading:
 **   http://en.wikipedia.org/wiki/Mutex
 **   http://en.wikipedia.org/wiki/Lock_(computer_science)
 **
 ** This wrapper uses a recursive mutex, which means that a thread can lock it
 ** multiple times.
 */
class Mutex {

protected:
	/// description/name of mutex
	std::string name;

	/// the actual (recursive) mutex
	std::recursive_timed_mutex mutex;


public:

	/** Constructor
	 */

	Mutex()
		: name("Unknown")
		, mutex()
	{}

	/** Destructor
	 */

	virtual ~Mutex() {
	}

	/// give the mutex a name
	void setName(const std::string &newName) {
		name = newName;
	}

	/// lock the mutex
	void lock();

	/// if the mutex can be locked, do it, otherwise return false
	inline bool trylock() {
		return mutex.try_lock();
	}

	/// unlock mutex
	inline void unlock() {
		mutex.unlock();
	}
};


/*------------------------------------------------------------------------------------------------*/

/**
 ** An Event object can be used to trigger actions for a waiting thread.
 **
 ** E.g. a thread may be waiting for data to arrive, and could use Event::wait to wait for another
 ** thread to signal this event.
 **
 ** Two event types are provided that differ in the way that a trigger will be handled. The 'normal'
 ** event will only trigger threads that are actually waiting at the moment that trigger() is called.
 ** The ContinuousEvent will keep the trigger active until reset() is called.
 */
class Event {
protected:
	std::condition_variable condition;

	std::mutex      mutex;
	int16_t         waiters;
	bool            triggered;

public:
	/// constructor
	Event()
		: condition()
		, mutex()
		, waiters(0)
		, triggered(false)
	{
	}

	/// destructor
	virtual ~Event() {
	}

	/** trigger the event, this will make any calls to wait() return, including
	 ** future calls until reset() is called
	 */
	virtual void trigger();

	/** wait for the event to be triggered (only call once!)
	 **
	 ** @param  timeout   Timeout in milliseconds
	 **
	 ** @return true if the event was triggered, false on timeout
	 */
	virtual bool wait(Millisecond timeout=0*milliseconds);
};


/*------------------------------------------------------------------------------------------------*/


/**
 ** A ContinuousEvent is similar to an Event, except that the trigger will be
 ** kept active until reset() is called.
 */
class ContinuousEvent : public Event {
public:
	virtual ~ContinuousEvent() {}

	/** trigger the event, this will make any calls to wait() return, including
	 ** future calls until reset() is called
	 */
	virtual void trigger();

	/** wait for the event to be triggered (only call once!)
	 **
	 ** @param   timeoutInMs   Timeout in milliseconds
	 **
	 ** @return true if the event was triggered, false on timeout
	 **
	 */
	virtual bool wait(Millisecond timeout=0*milliseconds);

	/** reset ('un'trigger) the event
	 */
	virtual void reset() {
		std::unique_lock<std::mutex> lock(mutex);
		triggered = false;
	}
};


/*------------------------------------------------------------------------------------------------*/

/**
 ** A critical section is basically a mutex that is used to prevent access to certain code areas
 ** for multiple threads at the same time. So for all sense and purposes it would be enough to
 ** just use the Mutex class for this. However in order to enforce having recursive locking in
 ** the Mutex and to have slightly better function names, a derived class has been created.
 **
 ** Also see http://en.wikipedia.org/wiki/Critical_section
 */

class CriticalSection {
private:
	mutable Mutex mutex;

public:
	CriticalSection() {}
	CriticalSection(const std::string &name) {
		mutex.setName(name);
	}

	/// set the name of the critical section (or rather the underlying mutex)
	void setName(const std::string &name) {
		mutex.setName(name);
	}

	/// enter critical section (identical to locking the mutex)
	inline void enter() const {
		mutex.lock();
	}

	/// leaving the critical section (identical to unlocking the mutex)
	inline void leave() const {
		mutex.unlock();
	}
};


/*------------------------------------------------------------------------------------------------*/

/**
 ** This is a small helper class to enter a critical section and leave it again
 ** when the instance is destroyed.
 */

class CriticalSectionLock {
protected:
	const CriticalSection &cs;

public:
	CriticalSectionLock(const CriticalSection &_cs)
		: cs(_cs)
	{
		cs.enter();
	}

	~CriticalSectionLock() {
		cs.leave();
	}
};


/*------------------------------------------------------------------------------------------------*/


#define THREAD_DEADLOCK_TERMINATE_TIMEOUT_SECONDS (20*seconds)

/**
 ** Wrapper for a thread
 **
 ** A thread is a lightweight process, in that it shares the address space of the creating process.
 ** On Linux, this is basically the main difference between a thread and a process.
 **
 ** On construction of a Thread object, the thread will not be automatically started, but a call
 ** to run() will have to be made. Consequently, the threadMain() function is running as the new
 ** thread.
 **
 ** Upon threadMain() returning, the thread is waiting to be joined. This will happen on destruction
 ** of the Thread object or when the cancel() function is called. Unless the thread is joined, its
 ** resources will not be freed.
 **
 ** This class was originally based on pthreads. For cross-platform compatibility reasons, it has
 ** been switched to C++11's new thread library.
 */

class Thread : virtual public NamedInstance {

private:
	/// thread object
	std::thread threadObject;

	// event to notify of start of thread
	ContinuousEvent threadStartEvent;

	// mutex and CV to notify of end of thread
	std::mutex quitMutex;
	std::condition_variable quitCV;

	static std::map<std::thread::id, std::string> threads;

	/// start time of thread
	Microsecond threadStartTime;

	/// is thread running?
	bool running;

	/// has thread stopped?
	bool hasStopped;

	/*
	 * Combinations running / hasStopped
	 *
	 *               false       true       Not yet started or has entirely quit
	 *               true        true       !!! should not happen !!!
	 *               false       false      Not yet fully initialized, or cancel() in progress
	 *               true        false      Still running
	 */

protected:
	/// thread main function, needs to be implemented by a thread
	virtual void threadMain() = 0;


public:
	/// constructor, creates a thread object and if run==true starts execution right away
	Thread();

	/// destructor, this will cancel the thread if it is still running
	virtual ~Thread();

	/// run this thread (only call once)
	void run();

	/// returns true if thread is presumably running
	inline bool isRunning() const { return running; }

	/// wait for thread to finish
	bool wait(Second timeout=-1*seconds, Second alreadyWaited=0);

	/// cancel this thread (only call once)
	void cancel(bool waitForCancelToFinish=true);

	/// set the thread's priority
	bool setNiceness(int nice=0);
	bool setRealTimePriority(int priority=0);

	/// print information about currently running thread
	static void printInfo();

	/// get name of currently running thread
	static std::string getCurrentThreadName();

	static CriticalSection threadRegistryCS;
};


#endif /* __THREAD_H__ */
