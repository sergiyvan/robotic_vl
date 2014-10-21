#include "utils/math/cholesky.h"

#include "debug.h"


/**
 * @brief Calculates the square root of this matrix using cholesky decomposition.
 *
 * See http://de.wikipedia.org/wiki/Cholesky-Zerlegung
 *
 * Requires matrix to be a positive definite symmetric matrix.
 * Sqrt-interpretation: (matrix) = G * G_transposed
 *
 * matrix = G * G.T
 *
 * @param matrix take the cholesky decomp. of this.
 *
 * @return result of the cholesky decomp.
 */
const arma::mat choleskySqrt(const arma::mat& matrix) {

	using namespace arma;
	using namespace std;

	const size_t N_ROWS = matrix.n_rows;
	const size_t N_COLS = matrix.n_cols;

	if (N_ROWS != N_COLS) {
		// TODO add exception
		//throw MVException(MVException::MatrixNotSquare);
		matrix.print();
		ERROR("Dimension of matix do not match. choleskySqrt");
	}

	mat result = zeros(N_ROWS, N_COLS);
	double tempSum = 0.;

	for (size_t row = 0; row < N_ROWS; row++) {
		for (size_t col = 0; col <= row ; col++) {

			tempSum = matrix(row, col);
			for (size_t k = 0; k < col; k++) {
				tempSum -= result(row, k) * result(col, k);
			}

			if ( row == col) {
				if (tempSum <= 0) {
					result(row, row) = 0.0001;
				} else {
					result(row, row) = sqrt(tempSum);
				}
			} else {
				result(row, col) = tempSum / result(col, col);
			}

		}//end for columns

	}// end for rows/row

	// TODO change algorithm in order to avoid trans()
	// maybe trans in not needed.
	return trans(result);
}
