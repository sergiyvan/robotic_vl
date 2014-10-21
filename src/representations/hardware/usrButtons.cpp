/*
 * usrButtons.cpp
 *
 *  Created on: Apr 26, 2013
 *      Author: lutz
 */

#include "usrButtons.h"


/*----------------------------------------------------------------------------*/
UsrButtons::UsrButtons() {
	for (auto& b : buttonStates) {
		b.currentState = false;
		b.duration     = DURATION_NONE;
		b.durationChange = false;
		b.pushType     = DURATION_NONE;
		b.pushStart    = 0*milliseconds;
		b.totalPushCount = 0;
		b.pushCount      = 0;
	}
}

/*----------------------------------------------------------------------------*/
UsrButtons::~UsrButtons() {
}

/*----------------------------------------------------------------------------*/
void UsrButtons::setInitButtonCount(std::array<uint16_t, Button::CT>& buttonCount) {
	for (int i(0); i<Button::CT; ++i) {
		buttonStates[i].totalPushCount = buttonCount[i];
	}
}

/*----------------------------------------------------------------------------*/
void UsrButtons::updateState(Button button, bool state) {

	buttonStates[button].pushType = DURATION_NONE;

	if (buttonStates[button].currentState != state) {
		if (buttonStates[button].currentState == false) {
			buttonStates[button].pushStart = getCurrentTime();
			buttonStates[button].duration = DURATION_NONE;
			buttonStates[button].currentState = true;
			buttonStates[button].pushType = DURATION_NONE;
		} else {
			buttonStates[button].currentState = false;
			buttonStates[button].pushType = buttonStates[button].duration;
			buttonStates[button].pushCount += 1;
			buttonStates[button].totalPushCount += 1;
		}
	}

	auto duration = getCurrentDuration(button);
	buttonStates[button].durationChange = buttonStates[button].duration != duration;
	buttonStates[button].duration = duration;

}
/*----------------------------------------------------------------------------*/
/** This Function is special made to deal with the information exchange
 *  between cognition and motion layer
 */
void UsrButtons::merge(const UsrButtons& button) {
	for (int i(0); i < Button::CT; ++i) {
		updateState(Button(i), button.buttonStates[i].currentState);
		buttonStates[i].totalPushCount = button.buttonStates[i].totalPushCount;
	}
}

/*----------------------------------------------------------------------------*/
/** Check whether a button was pushed
 **
 ** @param button           Button to check
 ** @param duration         which duration the button press should match
 **
 ** @return DURATION_NONE if button was not pressed, otherwise the classification of the duration
 */
bool UsrButtons::wasPushed(Button button, UsrButtonPushDuration duration) const {
	return buttonStates[button].pushType & duration;
}

/*----------------------------------------------------------------------------*/
/** get the number of times the button was pushed down
 **
 ** @param button  the button
 ** @param total   whether to count the pushes in the current cycle or since power-on
 ** @return the number of times the button was pushed
 */
uint16_t UsrButtons::getPushCount(Button button, bool total) const {
	if (total) {
		return buttonStates[button].totalPushCount;
	} else {
		return buttonStates[button].pushCount;
	}
}

/*----------------------------------------------------------------------------*/
UsrButtonPushDuration UsrButtons::getCurrentDuration(Button button) const {
	if (buttonStates[button].currentState == false) {
		return DURATION_NONE;
	} else if (buttonStates[button].pushStart + 2000*milliseconds < getCurrentTime()) {
		return DURATION_LONG;
	} else if (buttonStates[button].pushStart + 1000*milliseconds < getCurrentTime()) {
		return DURATION_NORMAL;
	} else {
		return DURATION_SHORT;
	}
}

/*----------------------------------------------------------------------------*/
bool UsrButtons::durationChange(Button button) const {
	return buttonStates[button].durationChange;
}

