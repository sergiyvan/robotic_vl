#ifndef GAUSSIANTOOLS_H
#define GAUSSIANTOOLS_H

#include "utils/math/Common.h"
#include <armadillo>


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Return the value of the probability density function of a multivariate gaussian.
 *
 * @param mean the mean of the Gaussian
 * @param Covariance the covariance matrix of the Gaussian
 * @param position the position of the calculation
 *
 * @return the value of the gaussian on the given position
 */
inline double multiVariateGaussian(const arma::colvec& mean,
                                   const arma::mat& Covariance,
                                   const arma::colvec& position)
{
	arma::colvec diff (mean - position);
	double exponent (arma::as_scalar(diff.t() * Covariance.i() * diff));
	double factor(1.0 / (Math::pi2 * sqrt(arma::det(Covariance))));
	return factor * exp(-0.5 * exponent);
}


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Computes the normalized probability for the given position.
 *
 * @param mean The mean of the Gaussian
 * @param Covariance The covariance matrix of the Gaussian
 * @param position The position of the calculation
 *
 * @return The normalized probability of the position.
 */
inline double normalizedProbabilityAt(const arma::colvec& mean,
                                      const arma::mat& Covariance,
                                      const arma::colvec& position)
{
	arma::colvec diff (mean - position);
	double exponent (arma::as_scalar(diff.t() * Covariance.i() * diff));
	return exp(-0.5 * exponent);
}


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Calculate the Mahalanobis distance of two gaussian distributions.
 *
 * You can use it to check if the distributions are the "same".
 *
 * See http://en.wikipedia.org/wiki/Mahalanobis_distance for more.
 *
 * @param mean1 mean of distribution 1
 * @param covariance1 covariance of distribution 1
 * @param mean2 mean of distribution 2
 * @param covariance2 covariance of distribution 2
 *
 * @return a distance metric (no unit)
 */
inline double mahalanobisDistance(const arma::colvec& mean1,
                                  const arma::mat& covariance1,
                                  const arma::colvec& mean2,
                                  const arma::mat& covariance2)
{
	arma::colvec diff(mean1 - mean2);
	arma::mat NewCov(covariance1 + covariance2);
	return arma::as_scalar(diff * (NewCov.i() * diff));
}

#endif /* GAUSSIANTOOLS_H */
