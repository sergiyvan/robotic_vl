#ifndef ROTATIONMATRIX_H
#define ROTATIONMATRIX_H

#include <armadillo>


/**
 * @brief Calculate a 2x2 rotation matrix.
 *
 * @param alpha the angle in radian to rotate around.
 *
 * @return the rotation matrix.
 */
inline arma::mat getRotationMatrix22(Radian alpha)
{
	arma::mat22 rotMat;
	rotMat << cos(alpha) << -sin(alpha) << arma::endr
	       << sin(alpha) << cos(alpha) << arma::endr;
	return rotMat;
}

#endif /* ROTATIONMATRIX_H */
