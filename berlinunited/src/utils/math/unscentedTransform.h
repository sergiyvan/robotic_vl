#ifndef UNSCENTEDTRANSFORM_H
#define UNSCENTEDTRANSFORM_H

#include <stdint.h>
#include <armadillo>


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Does the unscented transform which is used for Unscented KF.
 *
 * See "The Unscented Kalman Filter"
 */
class UnscentedTransform
{
public:
	UnscentedTransform(uint8_t stateDim = 3,
	                   double alpha = 1.f,
	                   double kappa = 0.,
	                   double beta = 2.);
	~UnscentedTransform() {};

	const arma::mat& transform(const arma::colvec& mean,
					const arma::mat& cov);


	arma::vec weights_mean;
	arma::vec weights_cov;

private:
	/* data */
	double lambda_;
	uint8_t STATE_DIM;
	uint8_t SIGMA_POINT_DIM;
	arma::mat SigmaPoints;
};

#endif /* UNSCENTEDTRANSFORM_H */
