/*
 * led.h
 *
 *  Created on: June 04, 2014
 *      Author: sgg
 */

#ifndef LEDSIGNALS_H
#define LEDSIGNALS_H


class LEDSignals {
public:
	virtual ~LEDSignals() {}

	virtual bool init() { return true; }

	virtual void setLED(int id, bool _active) {};
};

#endif

