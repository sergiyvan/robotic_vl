#ifndef RAWMAGDATAHISTORY_REPRESENTATION_H
#define RAWMAGDATAHISTORY_REPRESENTATION_H


#include "platform/system/timer.h"


/*------------------------------------------------------------------------------------------------*/

const uint8_t MAXMAGHISTORY = 100;


/*------------------------------------------------------------------------------------------------*/

/**
 * @brief Save the current and MAXGYROHISTORY last gyro values.
 */

class RawMagDataHistory {
public:
	RawMagDataHistory();
	virtual ~RawMagDataHistory() {};

	void addValue(robottime_t timestamp, float const* mag);
	void getValue(robottime_t timestamp, float* mag) const;

protected:
	struct MagValue {
		float mag[3];
		robottime_t timestamp;
	};

	// ringbuffer of gyro values
	MagValue magValues[MAXMAGHISTORY];

	// index to current value
	int index;
};

#endif
