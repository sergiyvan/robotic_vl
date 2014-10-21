#ifndef IMUFILTER
#define IMUFILTER

#include <armadillo>
#include "utils/units.h"


/**
 * ImuFilter - this class provides all functionality to use raw imu data
 * as accelerometer and gyroscope to optain a orientation as matrix/quaternion
 * (Using euler angles is not recommended" as it has gimbellock issues)
 */

class ImuFilter {
public:
	/**
	 * C'Tor
	 * @param _pInit value to initialize the covariance matrix with (e.g. 0.1)
	 */
	ImuFilter(double _pInit, double _qInit, double _rAccelInit);

	/**
	 * doing a predict step and applying drift compenstation
	 * @param _gyro angular velocity around the axis in radian per second
	 * @param _timestamp timestamp when these angular velocity was valid
	 */
	void predictWithGyro(std::array<RPS, 3> _gyro, Millisecond _timestamp);

	/**
	 * doing the update step
	 * @param _accel vector of the accelerometer
	 */
	void updateWithAccel(arma::colvec3 _accel);


	/**
	 * set an orientation offset that will be substracted from the end result.
	 * (you probably want to set this to "getRawQuaternion()"
	 */
	void setOrientationOffset(arma::colvec4 _quat);

	/**
	 * set the gyro drift
	 */
	void setGyroDrift(arma::colvec3 _gyroDrift);

	/**
	 * get the orientation without appying the orientattion offset.
	 * This is use full to set the orientation offset
	 */
	arma::colvec4 getRawQuaternion() const;


	/**
	 * get current orientation as quaternion
	 */
	arma::colvec4 getQuaternion() const;

	/**
	 * get current orientation as matrix
	 */
	arma::mat33   getMatrix() const;

	/**
	 * get current orientation as eular angles
	 * This should only be used for debugging.
	 */
	std::array<Radian, 3> getEulerAngles() const; // Roll, Pitch, Yaw
private:
	Millisecond timestamp;
	arma::colvec4 state;
	arma::mat33 P;

	arma::mat33 Raccel;
	arma::mat44 Q;

	arma::colvec3 gyroDrift;
	arma::colvec4 orientationOffset;
};

#endif
