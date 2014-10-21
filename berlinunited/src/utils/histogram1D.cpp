#include "histogram1D.h"
#include <cstdio>

Histogram1D::Histogram1D(int size, bool wrapAround)
: mHist(size, 0), mMinIndex(0), mMaxIndex(size-1), mWrapAround(wrapAround)
{
}

Histogram1D::Histogram1D(int min, int max, bool wrapAround)
: mHist(abs(min)+abs(max)+1, 0), mMinIndex(min), mMaxIndex(max), mWrapAround(wrapAround)
{
	mWrapAround = false;
}


Histogram1D::~Histogram1D() {
}

/**
 * this functions finds the cell with the maximum value (maxVal)
 * and integrates over all cell indices i within a range of -/+ integrationWidth
 *
 * @return pair <sum_i(val_i), sum_i(val_i * i)/sum_i(val_i)>
 */
std::pair<int, int> Histogram1D::maxValueAndIndex(MaximumMode m, int integrationWidth) {
	//	for (unsigned int i=0; i<mHist.size(); ++i){
	//		printf("%d ", mHist[i]);
	//		if((i+1)%10==0)
	//			printf("\n");
	//	}
	//	printf("\n");
	std::vector<int>::iterator max = std::max_element(mHist.begin(), mHist.end());
	const int maxVal = *max;
	const int maxIndex = static_cast<int>(std::distance(mHist.begin(), max));
	if(m == Peak) {
		return std::make_pair(maxVal, maxIndex + mMinIndex);
	}

	int sumValIndex = 0;
	int sumVal = 0;
	for (int i = maxIndex - integrationWidth; i <= maxIndex + integrationWidth; ++i) {
		int index = i;
		if (index < 0) {
			index += mHist.size();
		}
		if (index >= (int)mHist.size()) {
			index -= mHist.size();
		}
		int val = mHist[index];

		sumValIndex += val*i;
		sumVal += val;
	}
	int index = maxIndex + mMinIndex;
	if(sumVal > 0) {
		index = sumValIndex / sumVal + mMinIndex;
	}

	// due to the index shifts the result may be out of bounds
	if (mWrapAround) {
		if (index < mMinIndex) {
			index += mHist.size();
		}
		if (index > mMaxIndex) {
			index -= mHist.size();
		}
	}
	return std::make_pair(sumVal, index);
}

/**
 * Updates the histogram for the given value
 * @param val value to update
 * @param m update mode (Incerement1 or Increment121)
 * @param factor to add to the position of val in the histogram
 */
void Histogram1D::setValue(int val, UpdateMode m, int factor) {
	if (!mWrapAround && (val < mMinIndex || val > mMaxIndex))
		return;
	int index = (val - mMinIndex) % mHist.size();
	if (index < 0)
		index += mHist.size();
	if (m == Increment1) {
		mHist[index] += factor;
	}
	else { // Increment121
		mHist[index] += 2*factor;
		setValue(val-1, Increment1, factor);
		setValue(val+1, Increment1, factor);
	}
}

void Histogram1D::reset() {
	std::fill(mHist.begin(), mHist.end(), 0);
}

const std::vector<int>& Histogram1D::data() const {
	return mHist;
}
