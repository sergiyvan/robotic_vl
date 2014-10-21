#include "genericEKF.h"


/*----------------------------------------------------------------------------*/

GenericEKF::GenericEKF(arma::colvec const& _state, double _sigma)
 : state(_state) {
	Sigma = arma::eye(state.n_rows, state.n_rows)*_sigma;
}
GenericEKF::GenericEKF(arma::colvec const& _state, arma::mat const& _sigma)
 : state(_state)
 , Sigma(_sigma) {
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Do the prediction step of the EKF.
 *
 * Also called 'process update'.
 *
 * @param G Jacobian of g(state, control_u)
 * @param _control control movement
 * @param _dt time past since last correction
 * @param predicted_state state after applying the motionModel: g(state, controlU);
 */
void GenericEKF::predictEKF(arma::mat const& _G,
                                arma::colvec const& _control,
                                arma::mat const& _R) {
	state = _G * state + _control;
	Sigma = _G * Sigma * _G.t() + _R;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Do the correction step of the EKF.
 *
 * Also called 'measurement update'.
 *
 * @param _H Jacobian of the h(state)
 * @param _measurement_z The actual measurement.
 */
void GenericEKF::correctEKF(arma::mat const& _H,
                                arma::colvec const& _measurement_diff,
                                arma::mat const& _Q) {

	arma::mat I = arma::eye(Sigma.n_rows, Sigma.n_rows);
	arma::mat S = _H * Sigma * _H.t() + _Q;
	arma::mat K = Sigma * _H.t() * S.i();
	state = state + K * _measurement_diff;
	Sigma = (I - K * _H) * Sigma;

}

/*----------------------------------------------------------------------------*/
/**
 * Getter Methods
 */
arma::mat const& GenericEKF::getSigma() const {
	return Sigma;
}
arma::colvec const& GenericEKF::getState() const {
	return state;
}

/*----------------------------------------------------------------------------*/
void GenericEKF::setState(const arma::colvec& _state) {
	state = _state;
}

