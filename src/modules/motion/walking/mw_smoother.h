#ifndef MW_SMOOTHER_H
#define MW_SMOOTHER_H

/**
 * The smoothing class template can be used to get a mean of a specified number
 * of ints.
 */
template<const int size> class MW_Smoother {
public:
	/**
	 * Standard constructor.
	 * @return A new MW_SMoother object.
	 */
	MW_Smoother() ;

	/**
	 * The mean value of all size objects.
	 * @return The mean value of the last size objects.
	 */
	int getSmoothedValue();

	/**
	 * Adds a value. If already size values are saved the oldest is overwritten.
	 * @param value
	 */
	void addValue(const int value);
private:
	int values[size];
	int pos;
	int _sum;

};

#endif
