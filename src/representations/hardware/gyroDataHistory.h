#ifndef GYROHISTORY_REPRESENTATION_H_
#define GYROHISTORY_REPRESENTATION_H_

#include "gyroData.h"

#include "ModuleFramework/Serializer.h"

#include "platform/system/timer.h"


/*------------------------------------------------------------------------------------------------*/

const uint8_t MAXGYROHISTORY = 100;


/*------------------------------------------------------------------------------------------------*/

/**
 * @brief Save the current and MAXGYROHISTORY last gyro values.
 */

class GyroDataHistory {
public:
	GyroDataHistory();
	virtual ~GyroDataHistory() {};

	void addGyroValue(GyroData const& data);

	GyroData const& getGyroValue(robottime_t timestamp) const;

protected:
	// ringbuffer of gyro values
	GyroData gyroValues[MAXGYROHISTORY];

	// index to current value
	int index;

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & gyroValues;
		ar & index;
	}
};

REGISTER_SERIALIZATION(GyroDataHistory, 1)


#endif
