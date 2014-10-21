#include "rawMagDataHistory.h"

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

RawMagDataHistory::RawMagDataHistory()
	: index(0)
{
	// set history to empty
	memset(&magValues, 0, MAXMAGHISTORY*sizeof(magValues[0]));
}


/*------------------------------------------------------------------------------------------------*/

/** adding magnetometer data to the hisory
 *
 * @param robottime_t time of when the data was measured
 * @param mag is a pointer to a array of length 3, unit is gauss
 */
void RawMagDataHistory::addValue(robottime_t timestamp, float const* mag) {
	index = (index + 1) % MAXMAGHISTORY;

	magValues[index].timestamp = timestamp;
	memcpy(magValues[index].mag, mag, sizeof(float)*3);

}

/*------------------------------------------------------------------------------------------------*/

/**
 * @brief Find the mag values that are at least as old as the given request.
 *
 * Go backwards through the list until we either find an entry that is older
 * than requested, or we are back at the beginning
 *
 * @param timestamp identifies the time the gyro values were saved
 * @param mag must be a valid pointer to an array of length 3
 */
void RawMagDataHistory::getValue(robottime_t timestamp, float* mag) const {
	// magValues is used as a ring buffer
	// searching for the newest timestamp that is older then the given “timestamp“ parameter

	int oldestIndex = (index+1) % MAXMAGHISTORY;
	int i(index);
	while (i != oldestIndex) {
		if (magValues[i].timestamp < timestamp) {
			memcpy(mag, magValues[i].mag, sizeof(float)*3);
			break;
		}
		// next Step
		i = (i == 0 ? MAXMAGHISTORY-1 : i-1);
	}
}

/*------------------------------------------------------------------------------------------------*/
