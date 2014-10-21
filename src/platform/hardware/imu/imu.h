/*
 * imu.h
 *
 *  Created on: June 02, 2014
 *      Author: sgg
 */

#ifndef IMU_H
#define IMU_H

#include <array>
#include "utils/units.h"

class IMU {
public:
	virtual ~IMU() {}

	virtual bool init() { return true; }

	/* return quaternions */
	virtual std::array<double, 4> getOrientation() const {
		return {{1., 0., 0., 0.}};
	}

	/* return euler angles */
	virtual std::array<Degree, 3> getOrientationAsEulerAngles() const {
		return {{0*degrees, 0*degrees, 0*degrees}};
	}

	virtual std::array<DPS, 3> getGyroscopeData() const {
		return {{0*degrees_per_second, 0*degrees_per_second, 0*degrees_per_second}};
	}

	/* retrieve timestamp of current data */ // TODO: return as part of above functionss
	virtual Millisecond getTime() const {
		return 0*milliseconds;
	}
};

#endif

