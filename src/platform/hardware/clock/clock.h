/*
 * Clock.h
 *
 *  Created on: 19.10.2014
 *      Author: lutz
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include "utils/units.h"
#include "platform/system/timer.h"

class Clock {
public:
	virtual ~Clock() {
	}

	virtual Millisecond getCurrentTime() const {
		return ::getCurrentTime();
	}
};

#endif /* CLOCK_H_ */
