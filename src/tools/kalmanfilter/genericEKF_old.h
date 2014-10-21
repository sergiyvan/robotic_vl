#ifndef GENERICEKF_OLD_H
#define GENERICEKF_OLD_H

#include <armadillo>


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Generic Extended Kalman Filter to implement your own KF.
 *
 * Implements the basic math of an EKF.
 *
 * See "Probabilistic Robotics" for more. The names of the variables
 * are mostly consistent with the book, except where other names make
 * more sense.
 *
 * There are no seperate varialbles for the predicted state and covariance,
 * ie. state_t bel(state_t) etc.
 * This makes the equations simpler and the implementation more efficient.
 *
 * Convention:
 * - Matix --> CamelCase
 * - Vector --> lower_case
 *
 * How to use it:
 * - inherit from GenericEKF
 * - make sure to initialize I, Sigma, state according to your problem
 * - implement a function which
 *   -  calls predictEKF with appropriate arguments: G, R, predictedState,
 *      (This means you have to implement the process model.)
 *   -  calls correctEKF with appropriate arguments: H, Q, measurement_z,
 *      predicted_measurement
 *      (This means you have to implement the measurement model.)
 *
 * ======
 *  todo
 * ======
 * TODO add parameter for state dim and measurement dim in c'tor
 *      and initialize some vars
 *
 * TODO the functions don't really need the parameters. Q, R, H and G are
 *      member variables.
 *
 * TODO maybe add pure virtual functions to get Q, R, G, H etc.
 *
 */
class GenericEKF_old
{
public:
	GenericEKF_old() {};
	GenericEKF_old(arma::colvec _state);
	virtual ~GenericEKF_old() {};

	virtual void predictEKF(const arma::mat& G,
	                        const arma::mat& R,
	                        const arma::colvec& predicted_state);

	virtual void correctEKF(const arma::mat& H,
	                        const arma::mat& Q,
	                        const arma::colvec& measurement_z,
	                        const arma::colvec& predicted_measurement);

	virtual arma::mat calculateMeasurementCovS() const;

	void setState(arma::colvec const& _state);
	void setSigma(arma::mat const& _mat);

	arma::mat const&    getSigma() const;
	arma::colvec const& getState() const;

protected:

	/* data */
	// state related
	arma::colvec state; /// state (n): state of the KF
	arma::mat Sigma; /// Sigma (n*n): covariance of the KF

	// Jacobians
	arma::mat G; /// Jacobian for prediction G (n*n): state = G * stateOld
	arma::mat H; /// Jacobian for correction H (m*n): z = H * state

	// noise
	arma::mat R; /// process noise matrix in prediction step
	arma::mat Q; /// measurement noise matrix in correction step
};

#endif
