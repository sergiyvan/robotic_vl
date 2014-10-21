/*
 * buttons.h
 *
 *  Created on: June 04, 2014
 *      Author: sgg
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <array>

enum Button {
	ONE,
	RED = ONE,
	TWO,
	BLUE = TWO,
	THREE,
	YELLOW = THREE,
	FOUR,
	GREEN = FOUR,
	CT
};

class Buttons {
public:
	virtual ~Buttons() {}

	virtual bool init() { return true; }

	virtual bool isButtonPressed(Button _id) const { return false; }

	virtual std::array<uint16_t, Button::CT> getButtonPushCounts() const { return {{0}};}
};

#endif

