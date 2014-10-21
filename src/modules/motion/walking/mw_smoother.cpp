#include "mw_smoother.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

template<const int size>
MW_Smoother<size>::MW_Smoother() {
	_sum = 0;
	pos = 0;
	for(int i=0; i<size; ++i) {
		values[i] = 0;
	}
}

template<const int size>
int MW_Smoother<size>::getSmoothedValue() {
	int sum = _sum/size;
	return sum;
}

template<const int size>
void MW_Smoother<size>::addValue(int value) {
	pos++;
	if (pos == size) pos = 0;
	_sum = _sum - values[pos] + value;
	values[pos] = value;
}
