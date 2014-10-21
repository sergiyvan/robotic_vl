#include "gyroData.h"

GyroData::GyroData()
	: timestamp(0*milliseconds)
	, angles {{0*degrees, 0*degrees, 0*degrees}}
	, rotMat(arma::eye(3, 3))
{}

/*------------------------------------------------------------------------------------------------*/

GyroData::GyroData(Millisecond _timestamp, std::array<Degree, 3> _angles, arma::mat33 _rotMat, arma::colvec4 _quaternion)
 : timestamp(_timestamp)
 , angles(_angles)
 , rotMat(_rotMat)
 , quaternion(_quaternion) {
}

