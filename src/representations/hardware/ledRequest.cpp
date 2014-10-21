#include "ledRequest.h"

#include "services.h"
#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

LEDRequest::LEDRequest() {
	buttonLEDValues.fill(LED_OFF);
	buttonLEDValueRequests.fill(false);
}

/*------------------------------------------------------------------------------------------------*/

/**
 ** set all motor LEDs to a mode
 */

void LEDRequest::setAllMotorLEDS(LEDMode mode) {
	for (auto id : services.getRobotModel().getRobotDescription()->getMotorIDs()) {
		motorLEDValues[id]        = mode;
		motorLEDValueRequests[id] = true;
	}
}

/*------------------------------------------------------------------------------------------------*/

/**
 ** set a specific motor LED
 */
void LEDRequest::setMotorLED(MotorID motorID, LEDMode mode) {
	motorLEDValues[motorID]        = mode;
	motorLEDValueRequests[motorID] = true;
}

/*------------------------------------------------------------------------------------------------*/

/**
 ** set a button LED
 */
void LEDRequest::setButtonLED(Button buttonID, LEDMode mode) {
	buttonLEDValues[buttonID]        = mode;
	buttonLEDValueRequests[buttonID] = true;
}

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

LEDMode LEDRequest::getMotorLEDValue(MotorID _id) const {
	if (motorLEDValues.find(_id) == motorLEDValues.end()) {
		return LED_OFF;
	}

	return motorLEDValues.at(_id);
}

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

LEDMode LEDRequest::getButtonLEDValue(Button _id) const {
	return buttonLEDValues.at(_id);
}

/*------------------------------------------------------------------------------------------------*/

void LEDRequest::clear() {
	buttonLEDValueRequests.fill(false);
	for (auto& e : motorLEDValueRequests) {
		e.second = false;
	}
}

/*------------------------------------------------------------------------------------------------*/
//! TODO here is probably an error when merging led request between cognition
//and motion layer
void LEDRequest::merge(const LEDRequest& _ledRequest) {
	for (int i(0); i<Button::CT; ++i) {
		Button id = Button(i);
		if (_ledRequest.buttonLEDValueRequests.at(id)) {
			setButtonLED(id, _ledRequest.getButtonLEDValue(id));
		}
	}

	for (auto e : _ledRequest.motorLEDValueRequests) {
		if (e.second) {
			setMotorLED(e.first, _ledRequest.getMotorLEDValue(e.first));
		}
	}
}


