#include "ledWriter.h"
#include "modules/motion/motion.h"
#include "services.h"

#include "platform/hardware/actuators/actuators.h"
#include "platform/hardware/ledsignals/ledSignals.h"
#include "platform/hardware/robot/robotDescription.h"

REGISTER_MODULE(Motion, LEDWriter, true, "Sets the various LEDs");


#define LED_BLINK_INTERVAL_SLOW  (750*milliseconds)
#define LED_BLINK_INTERVAL       (500*milliseconds)
#define LED_BLINK_INTERVAL_FAST  (250*milliseconds)


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

LEDWriter::LEDWriter() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

LEDWriter::~LEDWriter() {
}


/*------------------------------------------------------------------------------------------------*/

/** Initialize the module. This is called by the framework when execute() is
 ** called for the first time.
 */

void LEDWriter::init() {
	// turn off all LEDs at startup
	buttonLEDStatus.fill(false);

	std::map<MotorID, bool> leds;
	for (auto id : services.getRobotModel().getRobotDescription()->getMotorIDs()) {
		leds[id] = false;
	}
	getHardware().getActuators()->setLED(leds);
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void LEDWriter::execute() {
	// Update all motor led values

	std::map<MotorID, bool> motorLeds;
	for (auto id : services.getRobotModel().getRobotDescription()->getMotorIDs()) {
		bool on = currentBlinkingState(getLEDRequest().getMotorLEDValue(id));
		if (   motorLEDStatus.find(id) == motorLEDStatus.end()
		    || on != motorLEDStatus.at(id))
		{
			motorLeds[id] = on;
			motorLEDStatus[id] = on;
		}
	}
	getHardware().getActuators()->setLED(motorLeds);

	// Update all button led values
	for (int i(0); i < Button::CT; ++i) {
		Button id = Button(i);
		updateButtonLED(id);
	}

}

/*------------------------------------------------------------------------------------------------*/

void LEDWriter::updateButtonLED(Button _id) {
	bool on = currentBlinkingState(getLEDRequest().getButtonLEDValue(_id));
	if (on != buttonLEDStatus.at(_id)) {
		getHardware().getLEDSignals()->setLED(_id, on);
		buttonLEDStatus[_id] = on;
	}
}

/*------------------------------------------------------------------------------------------------*/

/**
 ** computes current should state of a led in mode _mode
 */
bool LEDWriter::currentBlinkingState(LEDMode _mode) {
	bool on(false);

	int x = uint64_t(getCurrentTime().value())%1000;
	switch(_mode) {
	case LED_UNDEFINED:
	case LED_OFF:
		on = false;
		break;
	case LED_ON:
		on = true;
		break;
	case LED_BLINK_SLOW:
		on = uint64_t((getCurrentTime()/LED_BLINK_INTERVAL_SLOW).value())%2;
		break;
	case LED_BLINK:
		on = uint64_t((getCurrentTime()/LED_BLINK_INTERVAL).value())%2;
		break;
	case LED_BLINK_FAST:
		on = uint64_t((getCurrentTime()/LED_BLINK_INTERVAL_FAST).value())%2;
		break;
	case LED_SHORTBLINK_TYPE1:
		on = (x<100);
		break;
	case LED_SHORTBLINK_TYPE2:
		on = (x<100) || (x>200 && x<300);
		break;
	case LED_SHORTBLINK_TYPE3:
		on = (x<100) || (x>200 && x<300) || (x>400 && x<500);
		break;
	case LED_SHORTBLINK_TYPE4:
		on = (x<100) || (x>200 && x<300) || (x>400 && x<500) || (x>600 && x<700);
		break;
	default:
		break;
	}
	return on;
}

