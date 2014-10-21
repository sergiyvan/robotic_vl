/*
 * MotionTime.h
 *
 *  Created on: 19.10.2014
 *      Author: lutz
 */

#ifndef MOTIONTIME_H_
#define MOTIONTIME_H_

#include "utils/units.h"

class MotionTime {
public:
	MotionTime() {
	}

	virtual ~MotionTime()
	{ }

	Millisecond getCurTime() const {
		return m_curTime;
	}

	void setCurTime(Millisecond time) {
		m_curTime = time;
	}

private:
	Millisecond m_curTime;
};

#endif /* MOTIONTIME_H_ */
