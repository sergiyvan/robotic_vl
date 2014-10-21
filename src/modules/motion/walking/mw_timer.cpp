#include "mw_timer.h"

#include "services.h"
#include "management/config/config.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */


MW_Timer::MW_Timer()
	: start(getCurrentTime())
	, stopTime(0)
	, stoppedTime(0)
	, stopped(false)
	, cyclic(false)
	, stepsPerSecond(3.0)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

float MW_Timer::getTime(bool stop) {
	if (stopped)
		return stopTime;

	Millisecond time =  getCurrentTime() - start;
	float timeInDegree = (time.value()*(stepsPerSecond/5.555555)); //  5.555555 = (1000 ms / 180)
	if (stop && cyclic && timeInDegree >= 360) {
		stopTimer();
	}
	return timeInDegree;
}
