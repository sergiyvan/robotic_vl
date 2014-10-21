/** @file
 **
 ** Time related functions.
 **
**/

#ifndef _TIMER_INCLUDED_
#define _TIMER_INCLUDED_

#include <inttypes.h>

#include <thread>
#include <chrono>

#include "utils/units.h"


/*------------------------------------------------------------------------------------------------*/

typedef Millisecond robottime_t; // deprecated!


/*------------------------------------------------------------------------------------------------*/

/**
 ** returns the current time in milliseconds
 **
 ** @return number of milliseconds since epoch (i.e. Jan 1, 1970, 0:00 UTC)
**/

inline Millisecond getCurrentTime () {
	return
		Millisecond(
			std::chrono::duration_cast<std::chrono::microseconds>
				(std::chrono::system_clock::now().time_since_epoch()).count() * microseconds
		);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** returns a microsecond time value
 **
 ** @return number of microseconds since epoch (i.e. Jan 1, 1970, 0:00 UTC)
**/

inline Microsecond getCurrentMicroTime () {
	return std::chrono::duration_cast<std::chrono::microseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count() * microseconds;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Get the monotonic clock value. This clock cannot be set and represents
 ** monotonic time since some unspecified starting point (on Linux this is
 ** usually the uptime, however without suspend time). The advantage of this
 ** clock is that it never goes backwards. It is used, for example, in the
 ** UVC camera driver in V4L2.
 **
 ** @return number of milliseconds from the monotonic clock
 **
 */

inline Millisecond getMonotonicClock () {
	return
		Millisecond(
			std::chrono::duration_cast<std::chrono::microseconds>
				(std::chrono::steady_clock::now().time_since_epoch()).count() * microseconds
		);
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** delay execution
 **
 ** Sleeps for 'delayTime' seconds.
 **
 ** @param delayTime  Time to sleep in seconds.
**/

inline void delay(Second delayTimeInSeconds) {
	std::this_thread::sleep_for(std::chrono::seconds((long int)delayTimeInSeconds.value()));
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** delay execution
 **
 ** Sleeps for 'delayTime' milliseconds.
 **
 ** @param delayTime  Time to sleep in milliseconds.
**/

inline void delay(Millisecond delayTimeInMilliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds((long int)delayTimeInMilliseconds.value()));
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** delay execution
 **
 ** Sleeps for 'delayTime' microseconds.
 **
 ** @param delayTime  Time to sleep in microseconds.
**/

inline void delay(Microsecond delayTimeInMicroseconds) {
	std::this_thread::sleep_for(std::chrono::microseconds((long int)delayTimeInMicroseconds.value()));
}


/*------------------------------------------------------------------------------------------------*/

#endif
