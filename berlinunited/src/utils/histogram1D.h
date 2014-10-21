#ifndef HISTOGRAM1D_H_
#define HISTOGRAM1D_H_

#include <algorithm>
#include <vector>

/**
 * Defines a class for a 1D histogram
 */
class Histogram1D {
public:
	enum UpdateMode{
		Increment1,
		Increment121
	};

	enum MaximumMode{
		Peak,
		IntegratedPeak
	};

	Histogram1D(int size, bool wrapAround = true);
	Histogram1D(int min, int max, bool wrapAround = false);
	virtual ~Histogram1D();
	std::pair<int, int> maxValueAndIndex(MaximumMode m, int integrationWidth = 10);
	void setValue(int val, UpdateMode m, int factor=1);
	void reset();
	const std::vector<int>& data() const;

private:
	std::vector<int> mHist;
	int mMinIndex, mMaxIndex;
	bool mWrapAround;
};

#endif /* HISTOGRAM1D_H_ */
