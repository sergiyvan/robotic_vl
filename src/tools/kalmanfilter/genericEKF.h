#ifndef GENERICEKF_H
#define GENERICEKF_H

#include <armadillo>
#include "utils/units.h"

class GenericEKF
{
public:
	GenericEKF(arma::colvec const& _state, double _sigma=1);
	GenericEKF(arma::colvec const& _state, arma::mat const& _sigma);
	virtual ~GenericEKF() {};

	void predictEKF(arma::mat const& _G,
	                arma::colvec const& _control,
	                arma::mat const& _R);

	void correctEKF(arma::mat const& _H,
	                arma::colvec const& _measurement_diff,
	                arma::mat const& _Q);

	arma::mat const&    getSigma() const;
	arma::colvec const& getState() const;

	void setState(const arma::colvec& _state);
protected:

	// state related
	arma::colvec state; /// state (n): state of the KF
	arma::mat Sigma; /// Sigma (n*n): covariance of the KF
};

#endif
