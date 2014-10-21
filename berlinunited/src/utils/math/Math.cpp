#include "Math.h"

namespace Math {


/**
 * returns a gaussian distributed float (mean 0, deviation 1)
 *
 * algorithm taken from taygeta.com/random/gaussian.html
 *
 * @param mean
 * @param deviation
 * @return
 */
float randGaussFloat(float mean, float deviation) {
	float x1, x2, w;

	do {
		x1 = 2.0 * ((float) rand() / RAND_MAX) - 1.0;
		x2 = 2.0 * ((float) rand() / RAND_MAX) - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = sqrt( (-2.0 * log10( w ) ) / w );

	return mean + (x1 * w * deviation);
}

/**
 *
 * @param mean
 * @param deviation
 * @return
 */
int16_t randGaussInt(int16_t mean, int16_t deviation) {
	return static_cast<int16_t>(randGaussFloat(static_cast<float>(mean),static_cast<float>(deviation)));
}

}
