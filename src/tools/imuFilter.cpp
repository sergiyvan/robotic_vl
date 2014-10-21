#include "imuFilter.h"

ImuFilter::ImuFilter(double _pInit, double _qInit, double _rAccelInit) {

	timestamp = 0.*milliseconds;
	P = arma::eye(3, 3) * _pInit;

	state    = arma::zeros(4, 1);
	state(0) = 1.0;

	Q      = arma::eye(4, 4) * _qInit;
	Raccel = arma::eye(3, 3) * _rAccelInit;

	gyroDrift = arma::zeros(3, 1);
	orientationOffset    = arma::zeros(4, 1);
	orientationOffset(0) = 1.0;
}

void ImuFilter::predictWithGyro(std::array<RPS, 3> _gyro, Millisecond _timestamp) {

	// First run, we can't compute a timediff so we just return the function
	if (timestamp == 0.*milliseconds) {
		timestamp = _timestamp;
		return;
	}

	Millisecond timeDelta = _timestamp - timestamp;
	timestamp = _timestamp;


	// Convert radian/second to radian
	arma::colvec3 angle = arma::zeros(3, 1);
	for (int i(0); i<3 ; ++i) {
		angle(i) = (_gyro[i].value()+gyroDrift(i)) * Second(timeDelta).value();
	}


	/* Prediction:
	 * 1. Predict state: x_(k+1) = f(x_k) = x_k * w
	 * 2. Predict covariance: P_k = A_k*P_(k-1)*A_k' + Q_k
	 */
	// 1. Predict state
	float cv0 = std::cos(angle(0)*0.5);
	float cv1 = std::cos(angle(1)*0.5);
	float cv2 = std::cos(angle(2)*0.5);
	float sv0 = std::sin(angle(0)*0.5);
	float sv1 = std::sin(angle(1)*0.5);
	float sv2 = std::sin(angle(2)*0.5);

	arma::colvec4 w;
	w(0) = cv0*cv1*cv2 - sv0*sv1*sv2;
	w(1) = sv0*cv1*cv2 + cv0*sv1*sv2;
	w(2) = cv0*sv1*cv2 - sv0*cv1*sv2;
	w(3) = cv0*cv1*sv2 + sv0*sv1*cv2;


	// 2. Predict covariance
	arma::mat44 F;
	arma::mat44 Ft;
	F(0, 0) =  w(0);
	F(1, 0) =  w(1);
	F(2, 0) =  w(2);
	F(3, 0) =  w(3);

	F(0, 1) = -w(1);
	F(1, 1) =  w(0);
	F(2, 1) =  w(3);
	F(3, 1) = -w(2);

	F(0, 2) = -w(2);
	F(1, 2) = -w(3);
	F(2, 2) =  w(0);
	F(3, 2) =  w(1);

	F(0, 3) = -w(3);
	F(1, 3) =  w(2);
	F(2, 3) = -w(1);
	F(3, 3) =  w(0);
	Ft = F.t();

	//x = F * x;
	arma::colvec4 t_stat;
	state = F * state;

	//P = F*P*F'+Q
	P = F*P*Ft + Q*Second(timeDelta).value();
}
void ImuFilter::updateWithAccel(arma::colvec3 _accel) {
	arma::colvec3 accel = _accel / arma::norm(_accel, 2);
	accel(2) = -accel(2);

	/* Update Measurement:
	 * 0. H/Ht
	 * 1. S = (H*P*Ht + R)'
	 * 2. kalman gain: K_k = P*Ht * S
	 * 3. state estimate: x = x + K (z - H*x)
	 * 4. covariance update: P = (I - K*H)*P
	 */
	/* Update Measurement:
	 * 0. H
	 * 1. y = z - hx
	 * 2. S = H*P*H'+R
	 * 3. K = P*H'*S^-1
	 * 4. x = x+K*y
	 * 5. P = (I - K*H)*P
	 */
	// 0. H/Ht
	arma::mat H  = arma::zeros(3, 4);
	arma::mat Ht = arma::zeros(4, 3);
	{
		H(0, 0) =  2.0f*state(2);
		H(1, 0) = -2.0f*state(1);
		H(2, 0) =  2.0f*state(0);

		H(0, 1) =  2.0f*state(3);
		H(1, 1) = -2.0f*state(0);
		H(2, 1) = -2.0f*state(1);

		H(0, 2) =  2.0f*state(0);
		H(1, 2) =  2.0f*state(3);
		H(2, 2) = -2.0f*state(2);

		H(0, 3) =  2.0f*state(1);
		H(1, 3) =  2.0f*state(2);
		H(2, 3) =  2.0f*state(3);
		Ht = H.t();
	}

	// 1. y = z - Hx
	arma::colvec3 y = accel - H*state;

	// 2. S = H*P*H'+R
	arma::mat33 S = H*P*Ht + Raccel;

	// 3. K = P*Ht*S^-1
	arma::mat K = P*Ht*S.i();

	// 4. x = x+K*y
	state = state + K*y;
	state = state / arma::norm(state, 2);


	// 5. P = (I - K*H)*P
	P = (arma::eye(4, 4) - K*H)*P;
}
void ImuFilter::setOrientationOffset(arma::colvec4 _orientationOffset) {
	orientationOffset = _orientationOffset;
	orientationOffset.rows(1, 3) *= -1.0;
	orientationOffset = orientationOffset / arma::norm(orientationOffset, 2);
}
void ImuFilter::setGyroDrift(arma::colvec3 _gyroDrift) {
	gyroDrift = -_gyroDrift;
}

arma::colvec4 ImuFilter::getQuaternion() const {
	arma::colvec4 q;
	const arma::colvec4& q1 = state;
	const arma::colvec4& q2 = orientationOffset;

	q(0) = q1(0) * q2(0) - arma::dot(q1.rows(1, 3), q2.rows(1, 3));
	q.rows(1, 3) = q1(0) * q2.rows(1, 3) + q2(0) * q1.rows(1, 3) + arma::cross(q1.rows(1, 3), q2.rows(1, 3));
	return q;
}
arma::colvec4 ImuFilter::getRawQuaternion() const {
	return state;
}
arma::mat33 ImuFilter::getMatrix() const {
	arma::colvec4 q = getQuaternion();
	arma::mat33 rotMat;

	rotMat(0, 0) = 1-2*(q(2)*q(2) + q(3)*q(3));
	rotMat(1, 1) = 1-2*(q(1)*q(1) + q(3)*q(3));
	rotMat(2, 2) = 1-2*(q(1)*q(1) + q(2)*q(2));

	rotMat(0, 1) = -2*q(0)*q(3) + 2*q(1)*q(2);
	rotMat(0, 2) =  2*q(0)*q(2) + 2*q(1)*q(3);
	rotMat(1, 0) =  2*q(0)*q(3) + 2*q(1)*q(2);

	rotMat(1, 2) = -2*q(0)*q(1) + 2*q(2)*q(3);
	rotMat(2, 0) = -2*q(0)*q(2) + 2*q(1)*q(3);
	rotMat(2, 1) =  2*q(0)*q(1) + 2*q(2)*q(3);

	return rotMat;
}
std::array<Radian, 3> ImuFilter::getEulerAngles() const {
	arma::colvec4 q = getQuaternion();

	double sq1 = q(1)*q(1);
	double sq2 = q(2)*q(2);
	double sq3 = q(3)*q(3);

	std::array<Radian, 3> angles;
	angles[0] =  std::atan2( 2.0f * (q(0)*q(1) + q(2)*q(3)), 1 - 2*sq1 - 2*sq2) * radians;
	angles[1] =  std::asin(  2.0f * (q(0)*q(2) - q(3)*q(1))) * radians;
	angles[2] =  std::atan2( 2.0f * (q(0)*q(3) + q(1)*q(2)), 1 - 2*sq2 - 2*sq3) * radians;

	return angles;
}
