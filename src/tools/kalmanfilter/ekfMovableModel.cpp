#include "ekfMovableModel.h"

#include "utils/math/rotationMatrix.h"
#include "utils/math/Math.h"


arma::mat getVelocityMatrix(const arma::colvec2& velocity,
                            const arma::mat& rotMat,
                            Second dt,
                            double friction)
{
	double normv = arma::norm(velocity, 2);

	if (normv >= std::abs(friction * dt.value())) {
		return (1 + ((friction * dt.value()) / normv)) * rotMat;
	} else {
		return arma::zeros(2, 2);
	}
}

EkfMovableModel::EkfMovableModel(arma::colvec const& _state, arma::mat const& _R,
                                 arma::mat const& _Q, double _friction)
 : genericEKF(_state)
 , R(_R)
 , Q(_Q)
 , friction(_friction)
{
}

void EkfMovableModel::predictEKF(PositionRelative _move, Degree _rot, Second _dt)
{
	arma::colvec translationControl;
	translationControl << -_move.getX().value() << -_move.getY().value()
	                   << 0. << 0. << arma::endr;

	arma::mat22 rotMat = getRotationMatrix22(Radian(-_rot));

	arma::colvec2 velocity;
	velocity << genericEKF.getState()(2) << genericEKF.getState()(3) << arma::endr;


	// stateTransition matrix
	arma::mat stateTransition = arma::eye(4, 4);
	stateTransition.submat(0, 0, 1, 1) = rotMat;
	stateTransition.submat(0, 2, 1, 3) = rotMat * _dt.value();
	stateTransition.submat(2, 2, 3, 3) = getVelocityMatrix(velocity, rotMat, _dt, friction);

	genericEKF.predictEKF(stateTransition, translationControl, R);
}

void EkfMovableModel::correctEKF(arma::mat const& _measurement, Centimeter cameraHeight) {
	double x(genericEKF.getState()(0));
	double y(genericEKF.getState()(1));
	double r(cameraHeight.value());
	double x2(x * x);
	double y2(y * y);
	double r2(r * r);
	double muNorm = sqrt(x2 + y2);

	// H
	arma::mat H = arma::zeros(2, 4);
	H << -(r * x / (muNorm * (x2+y2+r2)))
	  << -(r * y / (muNorm * (x2+y2*r2)))
	  << 0. << 0.
	  << arma::endr
	  << -(y / (x2 + y2))
	  << (x / (x2 + y2))
	  << 0. << 0.
	  << arma::endr;
	arma::colvec expected_measurement_z;
	expected_measurement_z << atan2(r, muNorm) << atan2(y, x) << arma::endr;


	arma::colvec measurement_diff = _measurement - expected_measurement_z;
	measurement_diff(0) = Math::normalize(measurement_diff(0)*radians).value();
	measurement_diff(1) = Math::normalize(measurement_diff(1)*radians).value();
	genericEKF.correctEKF(H, measurement_diff, Q);
}
arma::mat const& EkfMovableModel::getSigma() const {
	return genericEKF.getSigma();
}
arma::colvec const& EkfMovableModel::getState() const {
	return genericEKF.getState();
}

arma::mat const& EkfMovableModel::getR() const {
	return R;
}
arma::mat const& EkfMovableModel::getQ() const {
	return Q;
}

