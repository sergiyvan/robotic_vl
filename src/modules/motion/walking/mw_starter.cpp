#include "mw_starter.h"

#include "platform/system/timer.h"

#include <algorithm>


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

double MW_Starter::getFactor() {
	double factor =  Millisecond(getCurrentTime() - _start).value() / (double)_length;
	return (double) std::min(factor, 1.0);
}

