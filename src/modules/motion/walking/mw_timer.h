#ifndef MW_TIMER_H_
#define MW_TIMER_H_

#include "platform/system/timer.h"

/**
 * The MW_Timer class gives the degree based time (e.g. from 0 to 360) of the feet
 * as well as the timers counting from the last impact.
 * Once initialized, they can reset by calling startTimer() or set to a specific
 * time by calling setTimer(int time).
 * <br/>
 * If a timer should stop automatically at 360 it must be made cyclic.
 */
class MW_Timer {
public:
	/**
	 * The standard constructor. This doesn't start the timer. startTimer() must
	 * be called afterwards.
	 * @return A new MW_Timer object.
	 */
	MW_Timer();
private:
	/**
	 * Returns the time of this specific timer in degrees since start. How long
	 * a degree is, is defined by the stepsPerSecond member, which can be set by
	 * the according setter function.
	 * @param stop If the timer is cyclic and stop is true the Time stops after
	 *             the call of this function, when the timer is after 360 degree.
	 *             The standard value is true. This version of the function is
	 *             private and thus the user can't set this parameter.
	 * @return The time in degrees since start of the timer.
	 */
	float getTime(bool stop);

public:
	/**
	 * Sets whether the timer is cyclic or not. If it is cyclic the timer stops
	 * increasing after the last call having a time bigger 360.
	 * @param cyclic true if the timer should be cyclic, false otherwise.
	 */
	void inline setCyclic(bool cyclic) {
		this->cyclic = cyclic;
	}

	/**
	 * Set the timer to a specific time.
	 * @param time The wished time.
	 */
	void inline setTimer(int time) {
		start = getCurrentTime() - ((time/stepsPerSecond) * 5.555555)*milliseconds;
	}

	/**
	 * Sets the timer to zero and restarts the timer if stopped before.
	 */
	void inline startTimer() {
		start = getCurrentTime();
		stopped = false;
	}
	/**
	 * Stops the timer. After this call getTime() will return the time when the
	 * timer was stopped.
	 */
	void inline stopTimer() {
		stopTime = getTime(false);
		stoppedTime = getCurrentTime();
		stopped = true;
	}

	/**
	 * Resumes the timer after a call to stopTimer.
	 */
	inline void restartTimer() {
		if (stopped) {
			stopped = false;
			start  += getCurrentTime() - stoppedTime;
		}
	}

	/**
	 * Returns the time in degrees since start of the timer. If the timer is cyclic
	 * after the first call returning a timer bigger/equal 360 the timer is stopped.
	 * @return The current time in degrees of this timer.
	 */
	inline float getTime() {
		return getTime(true);
	}

	/**
	 * Defines how long a degree is. Since 360° are equal to two steps a degree
	 * is computed as follows:
	 * \f$1 deg = \Delta t \cdot \frac{s}{\frac{1000ms}{180}}\f$
	 * where \f$\Delta t\f$ is the time in ms since start and \f$s\f$ is the
	 * constant set with this function (steps per second).
	 * @param steps
	 */
	void inline setStepsPerSecond(double steps) {
		stepsPerSecond = steps;
	}

private:
	robottime_t start; // time in milliseconds that timer started
	float stopTime; // time relative to start in 360°
	robottime_t stoppedTime; // time in milliseconds that timer stopped
	bool stopped;

	bool cyclic;
	double stepsPerSecond;
};

#endif
