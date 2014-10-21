/* 
 * File:   Stopwatch.h
 * Author: thomas
 *
 * Created on 18. April 2008, 18:00
 */

#ifndef _STOPWATCH_H
#define _STOPWATCH_H

#include <string>
#include <map>

#include <platform/system/timer.h>
#include <platform/system/thread.h>
#include <utils/patterns/singleton.h>

class StopwatchItem {
public:
	StopwatchItem()
		: option()
		, name()
		, start(0)
		, stop(0)
		, isValid(false)
		, mean(0*milliseconds)
		, n(0.0f)
		, min(Millisecond(10*seconds)) // start with 10 s :)
		, max(0*milliseconds)
		, lastValue(0)
	{}

	/** name of debug option */
	std::string option;

	/** name of the stopwatch */
	std::string name;

	/** starting time in micro-seconds */
	Microsecond start;

	/** stopping time in micro-seconds */
	Microsecond stop;

	/** true if the data (stop - start) is valid */
	bool isValid;

	// some statistics
	/** the mean time */
	Millisecond mean;
	/** count */
	float n;

	/** the minimum time */
	Millisecond min;
	/** the maximum time */
	Millisecond max;

	/** The last valid value */
	Millisecond lastValue;
};

class Stopwatch : public Singleton<Stopwatch> {
protected:
	friend class Singleton<Stopwatch>;
	Stopwatch();

public:
	virtual ~Stopwatch();

	const char* getName() const { return "StopWatch"; }

	// @Deprecated and shouldn't be used since access using this method is very slow
	void notifyStart(const std::string& option, const std::string& stopWatchName);

	// @Deprecated and shouldn't be used since access using this method is very slow
	void notifyStop(const std::string& option, const std::string& stopWatchName);

	/** */
	void notifyStart(StopwatchItem& stopwatchItem, const std::string& option, const std::string& stopWatchName);

	/** */
	void notifyStop(StopwatchItem& stopwatchItem, const std::string& option, const std::string& stopWatchName);

	/** */
	StopwatchItem& getStopwatchReference(const std::string& option, const std::string& stopWatchName);

	/** send out valid stopwatches for the option */
	void send(const std::string& option);

private:
	CriticalSection cs;

	typedef std::map<std::string, StopwatchItem>  StopwatchItems;
	typedef std::map<std::string, StopwatchItems> StopWatchOptions;

	StopWatchOptions options;
};

#endif  /* _STOPWATCH_H */
