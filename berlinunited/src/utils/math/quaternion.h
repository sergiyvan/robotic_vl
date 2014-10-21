#ifndef QUATERNION_H_
#define QUATERNION_H_

#include <armadillo>

template <class V> class Quaternion {
public:

	V r;       //!< real part
	V i, j, k; //!< imaginary parts


	/**
	 *
	 */
	Quaternion<V>() : r(0), i(0), j(0), k(0) {}

	/**
	 * Quaternion as 3d vector
	 * @param x
	 * @param y
	 * @param z
	 */
	Quaternion<V>(V x, V y, V z) : r(0), i(x), j(y), k(z) {}

	/**
	 *
	 * @param _r
	 * @param _i
	 * @param _j
	 * @param _k
	 */
	Quaternion<V>(V _r, V _i, V _j, V _k) : r(_r), i(_i), j(_j), k(_k) {}

	virtual ~Quaternion() {}

	inline arma::colvec3 getAsVector() const {
		arma::colvec3 vec;
		vec << i << j << k << arma::endr;
		return vec;
	}

	// overload operators

	inline Quaternion<V> operator+(const Quaternion<V> &other) const {
		return Quaternion<V>(r + other.r, i + other.i, j + other.j, k + other.k);
	}

	inline Quaternion<V>& operator+=(const Quaternion<V> &other) {
		r += other.r;
		i += other.i;
		j += other.j;
		k += other.k;

		return *this;
	}

	inline Quaternion<V> operator-(const Quaternion<V> &other) const {
		return Quaternion<V>(r - other.r, i - other.i, j - other.j, k - other.k);
	}

	inline Quaternion<V>& operator-=(const Quaternion<V> &other) {
		r -= other.r;
		i -= other.i;
		j -= other.j;
		k -= other.k;

		return *this;
	}

	inline Quaternion<V> operator*(const Quaternion<V> &other) const {
		Quaternion<V> mult;

		mult.r = r * other.r - i * other.i - j * other.j - k * other.k;
		mult.i = r * other.i + i * other.r + j * other.k - k * other.j;
		mult.j = r * other.j - i * other.k + j * other.r + k * other.i;
		mult.k = r * other.k + i * other.j - j * other.i + k * other.r;

		return mult;
	}

	inline Quaternion<V>& operator*=(const Quaternion<V> &other) {
		V newR = r * other.r - i * other.i - j * other.j - k * other.k;
		V newI = r * other.i + i * other.r + j * other.k - k * other.j;
		V newJ = r * other.j - i * other.k + j * other.r + k * other.i;
		V newK = r * other.k + i * other.j - j * other.i + k * other.r;

		r = newR;
		i = newI;
		j = newJ;
		k = newK;

		return *this;
	}

	inline Quaternion<V> operator*(V scalar) const {
		Quaternion<V> mult;

		mult.r = r * scalar;
		mult.i = i * scalar;
		mult.j = j * scalar;
		mult.k = k * scalar;

		return mult;
	}

	inline Quaternion<V>& operator*=(V scalar) {
		r *= scalar;
		i *= scalar;
		j *= scalar;
		k *= scalar;

		return *this;
	}

	inline Quaternion<V> operator/(V scalar) const {
		Quaternion<V> mult;

		mult.r = r / scalar;
		mult.i = i / scalar;
		mult.j = j / scalar;
		mult.k = k / scalar;

		return mult;
	}

	inline Quaternion<V>& operator/=(V scalar) {
		r /= scalar;
		i /= scalar;
		j /= scalar;
		k /= scalar;

		return *this;
	}

	/**
	 * Conjugate the quaternion
	 * @return
	 */
	inline Quaternion<V>& conjugate() {
		i = -i;
		j = -j;
		k = -k;

		return *this;
	}

	/**
	 * Calculates the inverse of the quaternion
	 * @return
	 */
	inline Quaternion<V>& inverse() {
		V n = norm();

		if (n == 1) { // test on Einheitsquaternion
			return conjugate();
		} else {
			conjugate();

			*this /= (n * n);

			return *this;
		}
	}

	/**
	 * Calculates the norm (length) of the quaternion
	 * @return
	 */
	inline V norm() const {
		return sqrt( scalarProduct(*this) );
	}

	/**
	 * Normalizes the quaternion (divide by norm)
	 * @return
	 */
	inline Quaternion<V>& normalize() {
		return *this / norm();
	}

	inline V scalarProduct(const Quaternion<V> &other) const {
		return r * other.r + i * other.i + j * other.j + k * other.k;
	}

	inline Quaternion<V> crossProduct(const Quaternion<V> &other) const {
		return (*this * other - other * *this) / 2;
	}

	/**
	 * Calculates the rotation around the axis given by (x, y, z) (norm of this vector must be 1!) by the specified angle.
	 * @param angleInRad
	 * @param x
	 * @param y
	 * @param z
	 * @return
	 */
	inline static Quaternion<V> calculateRotationQuaternion(V angleInRad, V x, V y, V z) {
		double angleHalf = angleInRad / 2.;
		double c = cos(angleHalf);
		double s = sin(angleHalf);

		return Quaternion<V>(c, x * s, y * s, z * s);
	}

	/**
	 * Rotates the quaternion (interpreted as 3d vector) around the axis (x, y, z) by the given angle.
	 * The length of the vector (x, y, z) must be 1!
	 * @param angleInRad
	 * @param x
	 * @param y
	 * @param z
	 * @return
	 */
	inline Quaternion<V> rotate(V angleInRad, V x, V y, V z) const {

		Quaternion<V> R = calculateRotationQuaternion(angleInRad, x, y, z);
		Quaternion<V> RInverse(R);
		RInverse.inverse();

		return R * *this * RInverse;
	}

	/**
	 * Rotates the quaternion by the given "rotation matrix" quaternion R
	 * @param R
	 * @return the rotatet quaternion
	 */
	inline Quaternion<V> rotate(const Quaternion<V> &R) {
		Quaternion<V> RInverse(R);
		RInverse.inverse();

		return R * *this * RInverse;
	}


};

typedef Quaternion<float> QuaternionF;

#endif /* QUATERNION_H_ */
