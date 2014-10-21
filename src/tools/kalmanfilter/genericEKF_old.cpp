#include "genericEKF_old.h"

GenericEKF_old::GenericEKF_old(arma::colvec _state) {
	state = _state;
	Sigma = arma::eye(state.n_rows, state.n_rows);
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Do the prediction step of the EKF.
 *
 * Also called 'process update'.
 *
 * @param G Jacobian of g(state, control_u)
 * @param R process noise matrix
 * @param predicted_state state after applying the motionModel: g(state, controlU);
 */
void GenericEKF_old::predictEKF(const arma::mat& G,
                            const arma::mat& R,
                            const arma::colvec& predicted_state)
{
	state = predicted_state;
	Sigma = G * Sigma * G.t() + R;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Do the correction step of the EKF.
 *
 * Also called 'measurement update'.
 *
 * @param H Jacobian of the h(state)
 * @param Q measurement noise matix
 * @param predicted_measurement h(state)
 * @param measurement_z The actual measurement.
 */
void GenericEKF_old::correctEKF(const arma::mat& H,
                            const arma::mat& Q,
                            const arma::colvec& measurement_z,
                            const arma::colvec& predicted_measurement)
{
	arma::mat I = arma::eye(Sigma.n_rows, Sigma.n_rows);

	arma::mat S = H * Sigma * H.t() + Q;
	arma::mat K = Sigma * H.t() * S.i();
	state = state + K * (measurement_z - predicted_measurement);
	Sigma = (I - K * H) * Sigma;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Calculate the measurement covariance S.
 *
 * S is handy to calculate likelihoods of correspondences.
 *
 */
arma::mat GenericEKF_old::calculateMeasurementCovS() const {
	return H * Sigma * H.t() + Q;
}

/*----------------------------------------------------------------------------*/
void GenericEKF_old::setState(arma::colvec const& _state) {
	state = _state;
}
/*----------------------------------------------------------------------------*/
void GenericEKF_old::setSigma(arma::mat const& _mat) {
	Sigma = _mat;
}

/*----------------------------------------------------------------------------*/
/**
 * Getter Methods
 */
arma::mat const& GenericEKF_old::getSigma() const {
	return Sigma;
}
arma::colvec const& GenericEKF_old::getState() const {
	return state;
}

