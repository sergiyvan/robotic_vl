#ifndef ODOMETRYDATA_H_
#define ODOMETRYDATA_H_

#include "ModuleFramework/Serializer.h"

#include "utils/units.h"
#include "tools/position.h"
#include "utils/math/Pose2D.h"
#include "utils/serializers/serializeArmadillo.h"


/*------------------------------------------------------------------------------------------------*/

/** \class OdometryData
 ** \brief representation of the robot's relative movement
 */

class OdometryData {
public:
	OdometryData() {
		clear();
		hasMoved = false;
	}

	void clear(bool _clrHasMoved = false) {
		translationX     = 0*centimeters;
		translationY     = 0*centimeters;
		rotation         = 0*degrees;
		rotationFiltered = 0*degrees;
		transitionMatrix = arma::eye(3, 3);
		if (_clrHasMoved) {
			hasMoved = false;
		}
	}

	void setTranslation(Centimeter _translationX, Centimeter _translationY) {
		translationX = _translationX;
		translationY = _translationY;
		transitionMatrix(0, 2) = -translationX.value();
		transitionMatrix(1, 2) = -translationY.value();
	}

	void setRotation(Degree _rotation) {
		rotation = _rotation;
		const double c = cos(-rotation);
		const double s = sin(-rotation);
		transitionMatrix(0, 0) = c;
		transitionMatrix(0, 1) = -s;
		transitionMatrix(1, 0) = s;
		transitionMatrix(1, 1) = c;
	}
	void setRotationFiltered(Degree _rotation) {
		rotationFiltered = _rotation;
	}
	void setHasMoved(bool _hasMoved) {
		hasMoved = _hasMoved;
	}

	Centimeter getTranslationX() const {
		return translationX;
	}

	Centimeter getTranslationY() const {
		return translationY;
	}

	Degree getRotation() const {
		return rotation;
	}
	Degree getRotationFiltered() const {
		return rotationFiltered;
	}
	bool getHasMoved() const {
		return hasMoved;
	}

	void applyToPosition(PositionRelative &pos) const {
		arma::colvec3 helper = arma::colvec({pos.getX().value(), pos.getY().value(), 1});
		helper = transitionMatrix * helper;
		pos = PositionRelative(helper(0)*centimeters, helper(1)*centimeters);
	}

	/**
	 * accumulates odometryData
	 *
	 * this is for the motion <-> cognition layer exchange
	 */
	void apply(OdometryData const& data) {
		Pose2D pose(Radian(rotation), translationX.value(), translationY.value());

		pose.translate(data.getTranslationX().value(), data.getTranslationY().value());
		pose.rotate((Radian)data.getRotation());


		setTranslation(pose.translation.x * centimeters, pose.translation.y * centimeters);
		setRotation(Degree(pose.rotation));
		rotationFiltered = Math::normalize(rotationFiltered + data.rotationFiltered);
		hasMoved     = data.hasMoved;
	}

	const arma::mat33 &getTransitionMatrix() const {
		return transitionMatrix;
	}


private:
	Centimeter  translationX;
	Centimeter  translationY;
	Degree      rotation;
	Degree      rotationFiltered;
	arma::mat33 transitionMatrix;
	bool hasMoved;

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & translationX;
		ar & translationY;
		ar & rotation;
		ar & rotationFiltered;
		::serializeArmadillo(ar, transitionMatrix);
		ar & hasMoved;

	}
};

REGISTER_SERIALIZATION(OdometryData, 1)

#endif
