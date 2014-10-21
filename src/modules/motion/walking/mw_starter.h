#ifndef MW_STARTER_H_
#define MW_STARTER_H_

#include "platform/system/timer.h"

/**
 * The MW_Starter class is a scaling class, which moves linearly from 0 to 1 in
 * a specific time. It is used for shortening the step height in the first steps.
 */

class MW_Starter {
public:
	/**
	 * The constructor. It also starts the starter. A restart can be triggered
	 * with start();
	 * @param length The duration until the starter is on 1.
	 * @return A new MW_Starter object, already moving.
	 */
	MW_Starter(int length)
		: _start(getCurrentTime()),
		  _length(length){}

	/**
	 * Restarts the starter. It starts at 0.
	 */
	inline void   start()     { _start = getCurrentTime(); }

	/**
	 * Returns the current factor in [0, 1].
	 * @return The current factor.
	 */
	double getFactor();

private:
	/**
	 * The time the starter was started.
	 */
	robottime_t _start;
	/**
	 * The duration until the starter reaches 1.
	 */
	int _length;
};

#endif
