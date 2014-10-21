/*
 * ClockODE.h
 *
 *  Created on: 19.10.2014
 *      Author: lutz
 */

#ifndef CLOCKODE_H_
#define CLOCKODE_H_

#include "clock.h"
#include "platform/system/thread.h"

class ClockODE : public Clock {
public:
	ClockODE();
	virtual ~ClockODE();

	virtual Millisecond getCurrentTime() const {
		CriticalSectionLock csl(m_cs);
		return m_curTime;
	}

	void setCurrentTime(Millisecond curTime) {
		CriticalSectionLock csl(m_cs);
		m_curTime = curTime;
	}

private:
	Millisecond m_curTime;
	CriticalSection m_cs;
};

#endif /* CLOCKODE_H_ */
