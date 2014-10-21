#ifndef HISTOGRAM2D_H_
#define HISTOGRAM2D_H_

#include <utility>
#include <vector>

/**
 * Defines a class for a 2D histogram
 */
class Histogram2D {
public:
	enum UpdateMode{
		Increment1,
		Increment121
	};
	Histogram2D(int min1, int max1, int min2, int max2);
	virtual ~Histogram2D();
	std::pair<int, std::pair<int, int> > maxValueAndIndex();
	void setValue(int val1, int val2, UpdateMode m, int factor = 1);
	void reset();
	const std::vector<int>& data() const;

private:
	int mMinIndex1;
	int mMinIndex2;
	int mSize1, mSize2;
	std::vector<int> mHist;
};

#endif /* HISTOGRAM2D_H_ */
