#include "utils/math/unscentedTransform.h"
#include <stdint.h>


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief C'tor
 *
 * Initialize values and
 * generate the weights that are needed for the unscented transform.
 *
 * See "the unscented kalman filter" page 6
 * @param stateDim
 * @param alpha spread of the sigma points arount the data. 1 <= alpha <= 1e-4
 * @param kappa scaling parameter: determine spread from the mean.
 *              Usally set to 0 or 3-STATE_DIM
 * @param beta  encode additional (higher order) knowledge about the dist
 *              2 is optimal if dist is Gaussian
 */
UnscentedTransform::UnscentedTransform(uint8_t stateDim,
                                       double alpha,
                                       double kappa,
                                       double beta)
	: weights_mean()
	, weights_cov()
	, lambda_(0.0)
	, STATE_DIM(stateDim)
	, SIGMA_POINT_DIM(2 * stateDim + 1)
	, SigmaPoints()
{
	SigmaPoints = arma::zeros(STATE_DIM, SIGMA_POINT_DIM);

	weights_mean = arma::zeros(SIGMA_POINT_DIM);
	weights_cov = arma::zeros(SIGMA_POINT_DIM);

	// GENERATE WEIGHS
	lambda_ = alpha * alpha * (STATE_DIM + kappa) - STATE_DIM;

	// the weights are pretty similar
	double tmp_weight = 1. / (2. * (STATE_DIM + lambda_));
	weights_mean.fill(tmp_weight);
	weights_cov.fill(tmp_weight);

	// and only differ in one spot
	weights_mean(0) = lambda_ / (STATE_DIM + lambda_);
	weights_cov(0) = lambda_ / (STATE_DIM + lambda_) + (1 - alpha * alpha + beta);
}


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Generate Sigma points.
 *
 * The weights are constant.  For more details see eq. 3.66 on page 65ff and
 * see "the unscented kalman filter" page 6.
 *
 * @param mean mean of which the sigma points get generated around
 * @param cov covariance in which the sigma points get generated
 */
const arma::mat& UnscentedTransform::transform(const arma::colvec& mean,
        const arma::mat& cov)
{
	using arma::mat;
	using arma::zeros;

	//mat sqrtCov = choleskySqrt((STATE_DIM + lambda_) * cov);
	mat sqrtCov = arma::chol((STATE_DIM + lambda_) * cov);
	sqrtCov.print("sqrtCov");

	SigmaPoints.col(0) = mean;
	for (size_t i = 0; i < STATE_DIM; i++) {
		// set 1 .. STATE_DIM+1
		SigmaPoints.col(i + 1)             = mean + sqrtCov.col(i);
		// set STATE_DIM+1 .. 2*STATE_DIM
		SigmaPoints.col(i + 1 + STATE_DIM) = mean - sqrtCov.col(i);
	}

	return SigmaPoints;
}
