#include "gyroDataHistory.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

GyroDataHistory::GyroDataHistory()
	: index(0)
{
	// set history to empty
	for (GyroData &gyroData : gyroValues)
	{
		gyroData = GyroData();
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Add a new gyro value to the list.
 **
 */

void GyroDataHistory::addGyroValue(GyroData const& data) {
	index = (index + 1) % MAXGYROHISTORY;

	gyroValues[index] = data;
}


/*------------------------------------------------------------------------------------------------*/

/**
 * @brief Find the gyro value data set that is at least as old as the given request.
 *
 * Go backwards through the list until we either find an entry that is older
 * than requested, or we are back at the beginning
 *
 * @param timestamp identifies the time the gyro values were saved
 */

GyroData const& GyroDataHistory::getGyroValue(robottime_t timestamp) const {
	// gyroValues is used as a ring buffer
	// searching for the newest timestamp that is older then the given “timestamp“ parameter
	//
	int oldestIndex = (index+1) % MAXGYROHISTORY;
	int i(index);
	while (i != oldestIndex) {
		if (gyroValues[i].getTimestamp() < timestamp) {
			return gyroValues[i];
		}

		// next Step
		i = (i == 0 ? MAXGYROHISTORY-1 : i-1);
	}

	// if nothing found, just return the last one - it's incorrect either way
	return gyroValues[index];
}
