#include "histogram2D.h"
#include <cstdlib>
#include <algorithm>
#include <cstdio>

Histogram2D::Histogram2D(int min1, int max1, int min2, int max2)
: mMinIndex1(min1),
  mMinIndex2(min2),
  mSize1(abs(min1)+abs(max1)+1), mSize2(abs(min2)+abs(max2)+1),
  mHist(mSize1*mSize2, 0)
{

}

Histogram2D::~Histogram2D() {
}

/**
 * Returns the maximum value of the histogram as well as the indices
 * @return pair of <value, <x,y> index>
 */
std::pair<int, std::pair<int, int> > Histogram2D::maxValueAndIndex(){
//	for (int i=0; i<61; ++i){
//		for (int j=0; j<61; ++j){
//			printf("%d ", mHist[i*61+j]);
//		}
//		printf("\n");
//	}
	std::vector<int>::iterator max = std::max_element(mHist.begin(), mHist.end());
	int index = static_cast<int>(std::distance(mHist.begin(), max));
	int index1 = index % mSize1;
	int index2 = index / mSize1;
	return std::make_pair(*max, std::make_pair(index1+mMinIndex1, index2+mMinIndex2));
}

/**
 * Adds a value to the <val1, val2> bin of the histogram
 * @param val1
 * @param val2
 * @param m UpdateMode
 * @param factor Factor to add to the selected bin (default is 1)
 */
void Histogram2D::setValue(int val1, int val2, UpdateMode m, int factor){
	int index1 = (val1 - mMinIndex1);
	int	index2 = (val2 - mMinIndex2);
	if(index1 < 0 || index1 >= mSize1 || index2<0 || index2 >= mSize2) {
		return;
	}
	int index = index1 + mSize1 * index2;
	if (m == Increment1){
		mHist[index] += factor;
	} else { //Increment121
		mHist[index] += 2*factor;
		setValue(val1-1, val2-1, Increment1, factor);
		setValue(val1-1, val2+0, Increment1, factor);
		setValue(val1-1, val2+1, Increment1, factor);
		setValue(val1+0, val2-1, Increment1, factor);
		setValue(val1+0, val2+1, Increment1, factor);
		setValue(val1+1, val2-1, Increment1, factor);
		setValue(val1+1, val2+0, Increment1, factor);
		setValue(val1+1, val2+1, Increment1, factor);
	}
}

/**
 * Resets the histogram
 */
void Histogram2D::reset(){
	std::fill(mHist.begin(), mHist.end(), 0);
}

/**
 *
 * @return raw data of the histogram
 */
const std::vector<int>& Histogram2D::data() const{
	return mHist;
}
