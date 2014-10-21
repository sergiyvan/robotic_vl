#include "debug.h"
#include "utils/math/Common.h"
#include <armadillo>

inline std::pair<arma::vec, arma::vec> getCovarianceMatrixAxes(const arma::mat& covariance) {
	if (covariance.n_cols == 0)
		return std::pair<arma::vec, arma::vec>(arma::vec2(), arma::vec2());

	// draw the covariance
	arma::mat U;
	arma::vec s;
	arma::mat V;
	arma::svd(U, s, V, covariance.submat(0, 0, 1, 1));

	// first main axis
	arma::vec px = sqrt(arma::as_scalar(s(0))) * V.row(0).t();

	// second main axis
	arma::vec py = sqrt(arma::as_scalar(s(1))) * V.row(1).t();

	return std::pair<arma::vec, arma::vec>(px, py);
}

/**
 * @brief Draw the covariance via the debuggin interface.
 *
 * @param debugName         the name of the debug option
 * @param state             the state to draw
 * @param covariance        the covariance to draw
 * @param red               used for visualization: 0--255
 * @param green             used for visualization: 0--255
 * @param blue              used for visualization: 0--255
 * @param imageDebugger       reference of the stream to draw on
 * @param drawStateAsCircle if true, draw the state as circle [default,
 *                          otherwise draw it as arrow. state(2) is
 *                          interpreted as angle in rad]
 */
inline void drawCovarianceMatrix(const std::string&  debugName,
                                 const arma::colvec& state,
                                 const arma::mat&    covariance,
                                 uint8_t             red,
                                 uint8_t             green,
                                 uint8_t             blue,
                                 bool                drawStateAsCircle=true)
{

	DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(debugName);
	if (_debug_option->enabled) {
		ImageDebugger& imageDebugger = Debugging::getInstance().getImageDebugger();

		imageDebugger.setColor(_debug_option, red, green, blue);

		// draw state
		if (drawStateAsCircle) {
			imageDebugger.addCircle(_debug_option, int16_t(state(0)), int16_t(state(1)), 5);
		}
		else {
			imageDebugger.addArrow(_debug_option,
					int16_t(state(0)), int16_t(state(1)), int16_t(Math::toDegrees(state(2))), 30);
		}

		// draw the covariance
		std::pair<arma::vec, arma::vec> v = getCovarianceMatrixAxes(covariance);

		// first main axis
		arma::vec p1 = state.subvec(0, 1) + v.first;
		arma::vec p2 = state.subvec(0, 1) - v.first;

		// second main axis
		arma::vec p3 = state.subvec(0, 1) + v.second;
		arma::vec p4 = state.subvec(0, 1) - v.second;

		/* using Math::round; */
		imageDebugger.addTetragon(_debug_option,
				int(round(p1(0))), int(round(p1(1))),
				int(round(p3(0))), int(round(p3(1))),
				int(round(p2(0))), int(round(p2(1))),
				int(round(p4(0))), int(round(p4(1))));
	}
}
