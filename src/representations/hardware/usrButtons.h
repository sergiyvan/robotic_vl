/*
 * usrButtons.h
 *
 *  Created on: Apr 26, 2013
 *      Author: lutz
 */

#ifndef USRBUTTONS_H
#define USRBUTTONS_H

#include <array>
#include "platform/hardware/buttons/buttons.h"
#include "utils/units.h"
#include "platform/system/timer.h"

typedef enum {
	DURATION_NONE = 0,
	DURATION_SHORT = 0x1,
	DURATION_NORMAL = 0x2,
	DURATION_LONG = 0x4,
	DURATION_ANY = 0xFF
} UsrButtonPushDuration;


class UsrButtons {
public:
	UsrButtons();
	virtual ~UsrButtons();

	void setInitButtonCount(std::array<uint16_t, Button::CT>& buttonCount);

	void updateState(Button button, bool state);
	void merge(const UsrButtons& button);

	// -- This is the function you want to use -- //
	bool wasPushed(Button button, UsrButtonPushDuration duration=DURATION_ANY) const;

	uint16_t getPushCount(Button button, bool total=true) const;

	UsrButtonPushDuration getCurrentDuration(Button button) const;
	bool durationChange(Button button) const;


private:

	struct ButtonState {
		bool currentState;               // current status of button (pushed or not pushed)
		UsrButtonPushDuration duration;  // current state of duration if triggered
		bool durationChange;             // get if duration has changed
		UsrButtonPushDuration pushType;  // current pushType
		Millisecond      pushStart;      // start of last button push
		uint16_t         totalPushCount; // number of times of pushes since power-on
		uint16_t         pushCount;      // number of times of pushes since last "cycle"
	};
	std::array<ButtonState, Button::CT> buttonStates;
};

#endif

