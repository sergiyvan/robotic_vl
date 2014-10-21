#ifndef LEDREQUEST_H_
#define LEDREQUEST_H_

#include "platform/system/timer.h"
#include "platform/hardware/buttons/buttons.h"
#include "platform/hardware/robot/motorIDs.h"

#include <array>


enum LEDMode {
	  LED_OFF
	, LED_ON
	, LED_UNDEFINED
	, LED_BLINK_SLOW
	, LED_BLINK
	, LED_BLINK_FAST
	, LED_SHORTBLINK_TYPE1
	, LED_SHORTBLINK_TYPE2
	, LED_SHORTBLINK_TYPE3
	, LED_SHORTBLINK_TYPE4
};


class LEDRequest {
public:
	LEDRequest();
	void setAllMotorLEDS(LEDMode mode);
	void setMotorLED(MotorID motorID, LEDMode mode);
	void setButtonLED(Button buttonID, LEDMode mode);

	LEDMode getMotorLEDValue(MotorID _id) const;
	LEDMode getButtonLEDValue(Button _id) const;

	void clear();

	void merge(const LEDRequest& _ledRequest);

private:
	std::array<LEDMode, Button::CT> buttonLEDValues;
	std::map<MotorID, LEDMode>      motorLEDValues;

	std::array<bool, Button::CT> buttonLEDValueRequests;
	std::map<MotorID, bool>      motorLEDValueRequests;
};

#endif

