#include "services.h"
#include "stopwatch.h"
#include "debugging/debugging.h"

#include "communication/comm.h"

#include <msg_debugging.pb.h>

#include <sstream>
#include <time.h>



/*------------------------------------------------------------------------------------------------*/

/** Constructor of Stopwatch singleton
 */

Stopwatch::Stopwatch()
	: options()
{
	cs.setName("Stopwatch");
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor of Stopwatch singleton
 */

Stopwatch::~Stopwatch() {
}


/*------------------------------------------------------------------------------------------------*/

/** Start a stopwatch.
 **
 ** This call requires a search for the stopwatch data, i.e. involves comparatively high overhead.
 **
 ** @param option           Name of debug option this stopwatch belongs to
 ** @param stopWatchName    Name of stopwatch
 */

void Stopwatch::notifyStart(const std::string& option, const std::string& stopWatchName) {
	notifyStart(getStopwatchReference(option, stopWatchName), option, stopWatchName);
}


/*------------------------------------------------------------------------------------------------*/

/** Stop the stopwatch.
 **
 ** This call requires a search for the stopwatch data, i.e. involves comparatively high overhead.
 **
 ** @param option           Name of debug option this stopwatch belongs to
 ** @param stopWatchName    Name of stopwatch
 */

void Stopwatch::notifyStop(const std::string& option, const std::string& stopWatchName) {
	StopWatchOptions::iterator option_it = options.find(option);
	if (option_it == options.end())
		return;

	StopwatchItems::iterator sw_it = (*option_it).second.find(stopWatchName);
	if (sw_it == (*option_it).second.end()) // could happen if option got enabled just now
		return;

	notifyStop((*sw_it).second, option, stopWatchName);
}


/*------------------------------------------------------------------------------------------------*/

/** Start a stopwatch.
 **
 ** It is checked that option and stopWatchName belong to the stopwatchItem. If not, the call
 ** does not use stopwatchItem, however this will cause additional overhead to locate the correct
 ** stopwatchItem.
 **
 ** @param stopwatchItem    Reference to a stopwatch, previously aquired with getStopwatchReference
 ** @param option           Name of debug option this stopwatch belongs to
 ** @param stopWatchName    Name of stopwatch
 */

void Stopwatch::notifyStart(StopwatchItem& stopwatchItem, const std::string& option, const std::string& stopWatchName) {
	if (stopwatchItem.option != option || stopwatchItem.name != stopWatchName) {
		notifyStart(option, stopWatchName);
		return;
	}

	stopwatchItem.isValid = false;
	stopwatchItem.start   = getCurrentMicroTime();
	stopwatchItem.stop    = stopwatchItem.start;
}


/*------------------------------------------------------------------------------------------------*/

/** Stop a stopwatch.
 **
 ** It is checked that option and stopWatchName belong to the stopwatchItem. If not, the call
 ** does not use stopwatchItem, however this will cause additional overhead to locate the correct
 ** stopwatchItem.
 **
 ** @param stopwatchItem    Reference to a stopwatch, previously aquired with getStopwatchReference
 ** @param option           Name of debug option this stopwatch belongs to
 ** @param stopWatchName    Name of stopwatch
 */

void Stopwatch::notifyStop(StopwatchItem& stopwatchItem, const std::string& option, const std::string& stopWatchName) {
	if (stopwatchItem.option != option || stopwatchItem.name != stopWatchName) {
		notifyStop(option, stopWatchName);
		return;
	}

	stopwatchItem.stop    = getCurrentMicroTime();
	stopwatchItem.isValid = true;

	// update the statistics of the item
	stopwatchItem.n++;
	Millisecond value = Millisecond(stopwatchItem.stop - stopwatchItem.start);

	stopwatchItem.min = std::min(stopwatchItem.min, value);
	stopwatchItem.max = std::max(stopwatchItem.max, value);

	// update the mean iteratively
	// c(n) = c(n-1) + (x_n - c(n-1))/n
	stopwatchItem.mean += (value - stopwatchItem.mean) / (double)stopwatchItem.n;

	stopwatchItem.lastValue = value;
}


/*------------------------------------------------------------------------------------------------*/

/** Retrieve a stopwatch by name.
 **
 ** @param option           Name of debug option this stopwatch belongs to
 ** @param stopWatchName    Name of stopwatch
 **
 ** @return stop watch reference. Not intended for direct use but as a parameter to start/stop.
 */

StopwatchItem& Stopwatch::getStopwatchReference(const std::string& option, const std::string& stopWatchName) {
	CriticalSectionLock lock(cs);

	StopWatchOptions::iterator option_it = options.find(option);
	if (option_it == options.end()) {
		options[option] = StopwatchItems();
		option_it = options.find(option);
	}

	StopwatchItems::iterator sw_it = (*option_it).second.find(stopWatchName);
	if (sw_it == (*option_it).second.end()) {
		(*option_it).second[stopWatchName] = StopwatchItem();
		sw_it = (*option_it).second.find(stopWatchName);

		(*sw_it).second.option = option;
		(*sw_it).second.name   = stopWatchName;
	}

	return (*sw_it).second;
}


/*------------------------------------------------------------------------------------------------*/

/** Send out data of all stopwatches belonging to a debug option.
 **
 ** @param option   Name of debug option
 */

void Stopwatch::send(const std::string& option) {
	// only handle existing options
	if (options.find(option) == options.end())
		return;

	// create option containter
	de::fumanoids::message::Message msg;
	msg.set_robotid(services.getID());

	de::fumanoids::message::Debugging &dbg = *msg.MutableExtension(de::fumanoids::message::debugging);
	de::fumanoids::message::Debugging::OptionStopwatch &sw = *dbg.add_optionstopwatch();

	sw.set_option(option);

	// send all valid stopwatches
	for (StopwatchItems::const_iterator iter = options[option].begin();
	     iter != options[option].end();
	     ++iter)
	{
		de::fumanoids::message::Debugging::OptionStopwatch::StopwatchValues &val = *sw.add_values();

		val.set_name(iter->second.name);
		val.set_duration(Millisecond(iter->second.lastValue).value());
		val.set_minimum(Millisecond(iter->second.min).value());
		val.set_maximum(Millisecond(iter->second.max).value());
		val.set_mean(Millisecond(iter->second.mean).value());
	}

	services.getComm().broadcastMessage(msg);
}
