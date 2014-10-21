#include "thread.h"
#include "signal.h"
#include "communication/comm.h"
#include "debug.h"

#include <thread>
#include <system_error>

#include <errno.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>


/*------------------------------------------------------------------------------------------------*/

CriticalSection Thread::threadRegistryCS;
std::map<std::thread::id, std::string> Thread::threads;


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor.
 **
 ** @param startThread   If set to true, the thread will be started right away. Otherwise you need to call run()
**/

Thread::Thread()
	: threadObject()
	, threadStartEvent()
	, quitMutex()
	, quitCV()
	, threadStartTime(0*microseconds)
	, running(false)   // we are not running
	, hasStopped(true) // as we are not running, we are stopped
{
//	printf("Thread object created: %lx\n", (long)this);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor. Aborts the thread if running
 */

Thread::~Thread() {
//	printf("Thread object destructor start: %lx\n", (long)this);
	if (running)
		cancel();

	if (threadObject.joinable()) {
		try {
			threadObject.join();
		} catch (std::system_error &e) {
			ERROR("Error while joining thread: %s", e.what());
		}
	}

	// TODO: remove thread from threads
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Start the thread. Guarantees that thread has started after run() returns.
 */

void Thread::run() {
//	printf("Going to run thread %s (%lx)\n", getName(), (long)this);

	// if we are already running, abort
	if (running) {
		ERROR("Tried to run thread %s (%lx) while it was already running", getName(), (long)this);
		return;
	}

	// indicate that thread is started (but not yet running)
	running = false;
	hasStopped = false;

	// create/start thread
	threadObject = std::thread([this]()
	{
		// we are running now
		running = true;
		threadStartTime = getCurrentMicroTime();

		// notify our parent that we started
		threadStartEvent.trigger();

		// register thread
		{
			CriticalSectionLock lock(threadRegistryCS);
			threads[std::this_thread::get_id()] = getName();
		}

		// thread main execution
		threadMain();

		// lock down and mark us stopped, also notify everybody who's waiting
		std::unique_lock<std::mutex> lock(quitMutex);
		running = false;
		hasStopped = true;
		quitCV.notify_all();
	});

	// wait for thread to start
	threadStartEvent.wait();

	// wait for thread to start
	while (!isRunning()) {
		std::this_thread::yield();
	}

//	printf("Thread object ready to run: %s (%lx)\n", getName(), (long)this);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Wait until thread has finished.
 **
 ** @param timeoutInSeconds Number of seconds to wait for thread to finish (-1 to wait indefinitely)
 **
 ** @return true if thread has finished, false otherwise
 */

bool Thread::wait(Second timeout, Second alreadyWaited) {
	// lock us down
	std::unique_lock<std::mutex> lock(quitMutex);

	// handle non-started threads or already expired ones
	if (this == 0 /* uh oh */ || false == threadObject.joinable() || hasStopped == true)
		return true;

	if (timeout.value() == -1) {
		quitCV.wait(lock, [this] { return hasStopped == true; });
	} else {
		if (false == quitCV.wait_until(lock, std::chrono::system_clock::now() + std::chrono::seconds((int)timeout.value()), [this]{return hasStopped == true;})) {
#ifdef ABORT_ON_DEADLOCK
			auto expired = (std::chrono::system_clock::now() - start) + std::chrono::seconds(alreadyWaited.value());
			if (expired > std::chrono::seconds(THREAD_DEADLOCK_TERMINATE_TIMEOUT_SECONDS)) {
				ERROR("ABORTING TO GET RID OF DEADLOCK");
				abort();
			}
#endif
			return false;
		}
	}

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Cancel the thread (if running)
 **^
 */

void Thread::cancel(bool waitForCancelToFinish) {
//	printf("Going to cancel thread %s (%lx)\n", getName(), (long)this);

	if (running) {
		running = false;
//		printf("Thread %s (%lx) set to non-running\n", getName(), (long)this);
		if (waitForCancelToFinish) {
			Second alreadyWaited = 0;
			while (!wait(10*seconds, alreadyWaited)) alreadyWaited += 10*seconds;
//			printf("Thread %s (%lx) has stopped\n", getName(), (long)this);
		}
	} else {
//		printf("Thread %s (%lx) was already cancelled\n", getName(), (long)this);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Set niceness of this thread.
 **
 ** Prerequisite: thread must be running
 **
 ** @param nice   Nice level to set, must be between -20 and 19. The lower the number the more
 **               CPU the thread will get. Default is nice=0.
 **
 ** @returns true on success
 */

bool Thread::setNiceness(int nice) {
	// lock us down, to prevent thread from quitting while we work on it natively
	std::unique_lock<std::mutex> lock(quitMutex);

	std::thread::native_handle_type tid = threadObject.native_handle();
	if (isRunning() == false || tid == 0) {
		ERROR("Can't set niceness of a thread that is not running (thread %s)", getName());
		return false;
	}

#ifdef __linux__
	if (0 == setpriority(PRIO_PROCESS, tid, nice))
		return true;
#endif

	return false;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Set realtime priority of this thread.
 **
 ** Note: any thread set with a priority will ALWAYS preempt a thread that did not have a priority set
 **       or which priority is lower
 **
 ** Prerequisite: thread must be running
 **
 ** @param priority  Priority to give the thread. Must be > 0.
 **
 ** @returns true on success
 */

bool Thread::setRealTimePriority(int priority) {
	// lock us down, to prevent thread from quitting while we work on it natively
	std::unique_lock<std::mutex> lock(quitMutex);

	if (false == isRunning() || false == threadObject.joinable())
		return false;

	std::thread::native_handle_type tid = threadObject.native_handle();
	if (tid == 0) {
		return false;
	}

#ifdef __linux__
	struct sched_param param = { priority };
	int policy = SCHED_RR;

	if (0 == pthread_setschedparam(tid, policy, &param))
		return true;
#endif

	return false;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Print information about currently running thread.
 **
 */

void Thread::printInfo() {
	printf("Thread: %s\n", getCurrentThreadName().c_str());
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Get name of currently running thread
 **
 */

std::string Thread::getCurrentThreadName() {
	CriticalSectionLock lock(threadRegistryCS);
	std::thread::id current_tid = std::this_thread::get_id();
	if (threads.find(current_tid) != threads.end())
		return threads[current_tid];
	else
		return "Unknown thread (main thread?)";
}


/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------------------------*/

/**
 ** locks the mutex
 **
 ** This call locks the mutex ("takes the resource"). If it is already locked
 ** (i.e. resource is already taken), this call blocks until the mutex can be
 ** locked.
 **
 ** @return true iff mutex is locked, false otherwise (e.g. timeout)
 */

void Mutex::lock() {
	const int timeout = 5;

	robottime_t startTime = getCurrentTime();
	robottime_t lastWaitTime = 0*milliseconds;

	while (true) {
		auto now = std::chrono::system_clock::now();
		bool success = mutex.try_lock_until(now + std::chrono::seconds(timeout));
		if (success)
			break;

		Millisecond waitTime = getCurrentTime() - startTime;

		if (waitTime > 4000*milliseconds && waitTime > lastWaitTime + 1000*milliseconds) {
			lastWaitTime = waitTime;
			ERROR("Potential deadlock, thread '%s' has been waiting %.0f seconds to acquire mutex %s (0x%lx).",
				Thread::getCurrentThreadName().c_str(),
				Second(waitTime).value(),
				name.c_str(),
				(long)this);

#ifdef ABORT_ON_DEADLOCK
			// if we waited for some time, just abort the program
			if (waitTime >= MUTEX_DEADLOCK_TERMINATE_TIMEOUT_SECONDS) {
				ERROR("ABORTING TO GET RID OF DEADLOCK");
				abort();
			}
#endif
		}
	}
}


/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/

void Event::trigger() {
	std::unique_lock<std::mutex> lock(mutex);

	// wake up all threads that are waiting
	triggered = true;
	condition.notify_all();

	// wait until everybody is awake
	while (waiters>0)
		condition.wait(lock); // this unlocks 'lock' on entry and locks it on exit

	// reset trigger
	triggered = false;

	// signal that new wait can start
	condition.notify_all();
}


/*------------------------------------------------------------------------------------------------*/

/** wait for the event to be triggered (only call once!)
 **
 ** @param  timeout   Timeout in milliseconds
 **
 ** @return true if the event was triggered, false on timeout
 */

bool Event::wait(Millisecond timeout) {
	bool timedOut = false;

	std::unique_lock<std::mutex> lock(mutex);

	// wait until we can start waiting
	while (triggered)
		condition.wait(lock); // this unlocks 'lock' on entry and locks it on exit

	waiters++;

	if (timeout > 0*milliseconds) {
		auto endTime = std::chrono::system_clock::now() + std::chrono::milliseconds((int)timeout.value());
		if (false == condition.wait_until(lock, endTime, [this]{return triggered;})) {
			timedOut = true;
		}
	} else {
		// wait for condition without timeout
		condition.wait(lock, [this]{return triggered;});
	}

	// finished waiting, and let the trigger know about it
	waiters--;
	condition.notify_all();

	return (false == timedOut);
}


/*------------------------------------------------------------------------------------------------*/

void ContinuousEvent::trigger() {
	std::unique_lock<std::mutex> lock(mutex);

	// wake up all threads that are waiting
	triggered = true;
	condition.notify_all();
}


/*------------------------------------------------------------------------------------------------*/

bool ContinuousEvent::wait(Millisecond timeout) {
	bool timedOut = false;

	std::unique_lock<std::mutex> lock(mutex);

	if (timeout > 0*milliseconds) {
		auto endTime = std::chrono::system_clock::now() + std::chrono::milliseconds((int)timeout.value());
		while (!triggered) {
			// wait for condition with timeout
			if (std::cv_status::timeout == condition.wait_until(lock, endTime)) {
				timedOut = true;
				break;
			}
		}
	} else {
		// wait for condition without timeout
		while (!triggered)
			condition.wait(lock);
	}

	// done
	return (false == timedOut);
}
