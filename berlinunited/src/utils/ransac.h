#ifndef RANSAC_H_
#define RANSAC_H_

#include <vector>
#include <float.h>

/**
 * Implements the RANSAC algorithm for a linear fitting.
 * The used datatype T needs to implement a function
 *     float T::distanceToModel(const T &m1, const T &m2) const;
 * where the current datapoint is matched with the given model of m1 and m2
 */
template<class T>
class Ransac {
public:
	Ransac() { }
	virtual ~Ransac() { }

	float ransac(int iterations);

	inline void setData(const std::vector<T> &data) {
		dataPoints = data;
	}

	inline T* getBestModel() {
		return bestFit;
	}

protected:
	std::vector<T> dataPoints;
	T bestFit[2];
};


/**
 *
 * @param iterations
 * @return
 */
template<class T>
float Ransac<T>::ransac(int iterations) {
	if(dataPoints.size() <= 2) { // we need at least 3 points for the fitting
		return 0.0f;
	}

	float bestError = FLT_MAX;
	for(int i = 0; i < iterations; ++i) {

		// choose randomly two points of the dataset
		int index1 = rand() % dataPoints.size();
		int index2 = rand() % dataPoints.size();

		if(index1 == index2) {
			index2 = (index1 + dataPoints.size() / 2) % dataPoints.size();
		}

		const T &p1 = dataPoints.at(index1);
		const T &p2 = dataPoints.at(index2);

		// calculate distance of the set to the model formed from the two points
		float errorAcc = 0.0f;
		uint8_t pointsCounter = 0;
		for(typename std::vector<T>::const_iterator iter = dataPoints.begin(); iter != dataPoints.end(); ++iter) {
			const T &dataPoint = *iter;
			if(dataPoint == p1 || dataPoint == p2) {
				continue;
			}

			errorAcc += dataPoint.distanceToModel(p1, p2);
			pointsCounter++;
		}

		if(pointsCounter != 0) {
			errorAcc = errorAcc / (float) pointsCounter;
		}

		// we have a better fit to the dataset
		if(errorAcc < bestError) {
			bestError = errorAcc;
			bestFit[0] = p1;
			bestFit[1] = p2;
		}
	}

	return bestError;

}

#endif /* RANSAC_H_ */
