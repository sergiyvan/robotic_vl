#ifndef GYRODATA_H__
#define GYRODATA_H__

#include "ModuleFramework/Serializer.h"

#include "utils/units.h"
#include "utils/serializers/serializeArmadillo.h"

#include <armadillo>
#include <array>

/**
 *
 **/

class GyroData {
public:

	GyroData();
	GyroData(Millisecond _timestamp, std::array<Degree, 3> _angles, arma::mat33 _rotMat, arma::colvec4 _quaternion);

	bool isValid() const {
		return timestamp.value() != 0;
	}
	robottime_t getTimestamp() const {
		return timestamp;
	}

	Degree getPitch() const {
		return angles[0];
	}

	Degree getRoll() const {
		return angles[1];
	}

	Degree getYaw() const {
		return angles[2];
	}

	arma::mat33 const &getRotMat() const {
		return rotMat;
	}

	arma::colvec4 getQuaternion() const {
		return quaternion;
	}
protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & timestamp;
		ar & boost::serialization::make_array(angles.data(), angles.size());
		serializeArmadillo(ar, rotMat);
	}

private:
	// estimated time these valids were actually valid
	Millisecond timestamp;

	std::array<Degree, 3> angles; //0. pitch, 1. roll, 2. yaw
	arma::mat33 rotMat;
	arma::colvec4 quaternion;
};

REGISTER_SERIALIZATION(GyroData, 1)


#endif

