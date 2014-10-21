/*
 * footCoordinateFrame.h
 *
 *  Created on: 31.03.2014
 *      Author: lutz
 */

#ifndef FOOTCOORDINATEFRAME_H_
#define FOOTCOORDINATEFRAME_H_

#include <utils/units.h>
#include <tools/position.h>

class FootCoordinateFrame {
public:
	FootCoordinateFrame()
		: leftFootForwardTransform(arma::eye(3, 3))
		, leftFootBackwardTransform(arma::eye(3, 3))
		, rightFootForwardTransform(arma::eye(3, 3))
		, rightFootBackwardTransform(arma::eye(3, 3))
	{}

	virtual ~FootCoordinateFrame() {}

	PositionRelative transformRelToLeftFoot(const PositionRelative &relPos) const {
		arma::colvec3 helper;
		helper(0) = relPos.getX().value();
		helper(1) = relPos.getY().value();
		helper(2) = 1.;

		helper = leftFootBackwardTransform * helper;

		return PositionRelative(helper(0) * centimeters, helper(1) * centimeters);
	}

	PositionRelative transformLeftFootFrameToRel(const PositionRelative &relPos) const {
		arma::colvec3 helper;
		helper(0) = relPos.getX().value();
		helper(1) = relPos.getY().value();
		helper(2) = 1.;

		helper = leftFootForwardTransform * helper;

		return PositionRelative(helper(0) * centimeters, helper(1) * centimeters);
	}

	void setParamsLeftFoot(arma::mat33 forwardMatrix) {
		leftFootForwardTransform = forwardMatrix;
		leftFootBackwardTransform = arma::inv(leftFootForwardTransform);
	}

	PositionRelative transformRelToRightFoot(const PositionRelative &relPos) const {
		arma::colvec3 helper;
		helper(0) = relPos.getX().value();
		helper(1) = relPos.getY().value();
		helper(2) = 1.;

		helper = rightFootBackwardTransform * helper;

		return PositionRelative(helper(0) * centimeters, helper(1) * centimeters);
	}

	PositionRelative transformRightFootFrameToRel(const PositionRelative &relPos) const {
		arma::colvec3 helper;
		helper(0) = relPos.getX().value();
		helper(1) = relPos.getY().value();
		helper(2) = 1.;

		helper = rightFootForwardTransform * helper;

		return PositionRelative(helper(0) * centimeters, helper(1) * centimeters);
	}

	void setParamsRightFoot(arma::mat33 forwardMatrix) {
		rightFootForwardTransform = forwardMatrix;
		rightFootBackwardTransform = arma::inv(rightFootForwardTransform);
	}

private:
	arma::mat33 leftFootForwardTransform, leftFootBackwardTransform;
	arma::mat33 rightFootForwardTransform, rightFootBackwardTransform;

};

#endif /* FOOTCOORDINATEFRAME_H_ */
