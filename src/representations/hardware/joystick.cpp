#include "joystick.h"


/*------------------------------------------------------------------------------------------------*/

Joystick::~Joystick() {
	if (axisData)
		free(axisData);

	if (buttonData)
		free(buttonData);

	if (buttonTriggerData)
		free(buttonTriggerData);
}


/*------------------------------------------------------------------------------------------------*/

/** Set the model name and adjust the mappings.
 **
 ** Annoyingly different gamepads have their buttons differently numbered. This
 ** makes it kinda hard to assign a specific button a functionality, if this
 ** button is at a different place (e.g. left instead of right) on another pad.
 **
 ** We assume a "default" gamepad, and for all the other gamepads we have in our
 ** lab we adjust the mapping so they are all the same.
 */

void Joystick::setModel(const std::string &modelName, uint8_t axisCount, uint8_t buttonCount) {
	this->modelName   = modelName;
	this->axisCount   = axisCount;
	this->buttonCount = buttonCount;

	axisData          = (int32_t *)calloc(axisCount,   sizeof(int32_t));
	buttonData        = (char *)   calloc(buttonCount, sizeof(char));
	buttonTriggerData = (char *)   calloc(buttonCount, sizeof(char));

	/*
	 * Map buttons and axis to our definitions. Use jstest-gtk to determine the
	 * values for new devices.
	 */

	if (modelName == "Xbox 360 Wireless Receiver") {
		/*
		Button  0: 304 (A, down)
		Button  1: 305 (B, right)
		Button  2: 307 (X, left)
		Button  3: 308 (Y, up)
		Button  4: 310 (left trigger)
		Button  5: 311 (right trigger)
		Button  6: 314 (back)
		Button  7: 315 (start)
		Button  8: 316 (big blinking button)
		Button  9: 317 (left joystick pressed)
		Button  10: 318 (right joystick pressed)
		Button  11: 704 (left)
		Button  12: 705 (right)
		Button  13: 706 (top)
		Button  14: 707 (down)

		Axis 0: left horizontal (left = negative, right = positive)
		Axis 1: left vertical (up = negative, down = positive)
		Axis 2: left trigger (-32k to +32k)
		Axis 3: right horizontal (left = negative, right = positive)
		Axis 4: right vertical (up = negative, down = positive)
		Axis 5: right trigger (-32k to +32k)
		*/

		buttonMapping[BUTTON_LEFT]                = 2;
		buttonMapping[BUTTON_UP]                  = 3;
		buttonMapping[BUTTON_RIGHT]               = 1;
		buttonMapping[BUTTON_DOWN]                = 0;
		buttonMapping[BUTTON_LEFTTRIGGER_TOP]     = 4;
		buttonMapping[BUTTON_RIGHTTRIGGER_TOP]    = 5;
		buttonMapping[BUTTON_BACK]                = 6;
		buttonMapping[BUTTON_START]               = 7;
		buttonMapping[BUTTON_DPAD_LEFT]           = 11;
		buttonMapping[BUTTON_DPAD_UP]             = 13;
		buttonMapping[BUTTON_DPAD_RIGHT]          = 12;
		buttonMapping[BUTTON_DPAD_DOWN]           = 14;
		buttonMapping[BUTTON_LEFTJOYSTICK]        = 9;
		buttonMapping[BUTTON_RIGHTJOYSTICK]       = 10;

		axisMapping[LEFTHAND_HORIZONTAL]  = 0 + 1;
		axisMapping[LEFTHAND_VERTICAL]    = 1 + 1;
		axisMapping[RIGHTHAND_HORIZONTAL] = 3 + 1;
		axisMapping[RIGHTHAND_VERTICAL]   = 4 + 1;
		axisMapping[LEFTTRIGGER]          = 2 + 1;
		axisMapping[RIGHTTRIGGER]         = 5 + 1;
	}
	else if (modelName == "Jess Tech Dual Analog Pad") {
		/*
		Button  0: 288 (left)
		Button  1: 289 (up)
		Button  2: 290 (down)
		Button  3: 291 (right)
		Button  4: 292 (left trigger, top)
		Button  5: 293 (left trigger, bottom)
		Button  6: 294 (right trigger, top)
		Button  7: 295 (right trigger, bottom)
		Button  8: 296 (back)
		Button  9: 297 (start)
		Button  10: 298 (left joystick pressed)
		Button  11: 299 (right joystick pressed)

		LED ON (default):
		Axis 0: left horizontal (left = negative, right = positive)
		Axis 1: left vertical (up = negative, down = positive)
		Axis 2: right vertical (up = negative, down = positive)
		Axis 3: right horizontal (left = negative, right = positive)
		Axis 4: dpad left (negative) / right (positive)
		Axis 5: dpad up (negative) / right (positive)

		LED OFF:
		Axis 0: dpad left (negative) / right (positive)
		Axis 1: dpad up (negative) / right (positive)
		Axis 2: right vertical (up = negative, down = positive)
		Axis 3: right horizontal (left = negative, right = positive)
		Axis 4: left joystick - left (negative) / right (positive)
		Axis 5: left joystick - up (negative) / down (positive)
		*/

		buttonMapping[BUTTON_LEFT]                = 0;
		buttonMapping[BUTTON_UP]                  = 1;
		buttonMapping[BUTTON_RIGHT]               = 3;
		buttonMapping[BUTTON_DOWN]                = 2;
		buttonMapping[BUTTON_LEFTTRIGGER_TOP]     = 4;
		buttonMapping[BUTTON_LEFTTRIGGER_BOTTOM]  = 5;
		buttonMapping[BUTTON_RIGHTTRIGGER_TOP]    = 6;
		buttonMapping[BUTTON_RIGHTTRIGGER_BOTTOM] = 7;
		buttonMapping[BUTTON_BACK]                = 8;
		buttonMapping[BUTTON_START]               = 9;
		buttonMapping[BUTTON_LEFTJOYSTICK]        = 10;
		buttonMapping[BUTTON_RIGHTJOYSTICK]       = 11;

		axisMapping[LEFTHAND_HORIZONTAL]  = 0 + 1;
		axisMapping[LEFTHAND_VERTICAL]    = 1 + 1;
		axisMapping[RIGHTHAND_HORIZONTAL] = 3 + 1;
		axisMapping[RIGHTHAND_VERTICAL]   = 4 + 1;
		axisMapping[LEFTTRIGGER]          = 2 + 1;
		axisMapping[RIGHTTRIGGER]         = 5 + 1;
	}
	else if (modelName == "DragonRise Inc.   Generic   USB  Joystick  " /* do not adjust spaces! */) {
		/*
		Button  0: 288 (up)
		Button  1: 289 (right)
		Button  2: 290 (down)
		Button  3: 291 (left)
		Button  4: 292 (left trigger, top)
		Button  5: 293 (right trigger, top)
		Button  6: 294 (left trigger, bottom)
		Button  7: 295 (right trigger, bottom)
		Button  8: 296 (back)
		Button  9: 297 (start)
		Button  10: 298 (left joystick pressed)
		Button  11: 299 (right joystick pressed)

		LED ON:
		Axis 0: left joystick horizontal (left = negative, right = positive)
		Axis 1: left joystick vertical (up = negative, down = positive)
		Axis 2: left joystick horizontal and right vertical (left = negative, right = positive)
		Axis 3: right joystick horizontal (left = negative, right = positive)
		Axis 4: right joystick vertical (up = negative, down = positive)
		Axis 5: dpad left (negative), right (positive)
		Axis 6: dpad up (negative), down (positive)

		LED OFF (default):
		Axis 0: left joystick and dpad (left = negative, right = positive) discrete
		Axis 1: left joystick and dpad (up = negative, down = positive) discrete
		Axis 2: left joystick horizontal and right vertical (left = negative, right = positive)
		Axis 3:
		Axis 4:
		Axis 5:
		Axis 6:
		 */

		buttonMapping[BUTTON_LEFT]                = 3;
		buttonMapping[BUTTON_UP]                  = 0;
		buttonMapping[BUTTON_RIGHT]               = 1;
		buttonMapping[BUTTON_DOWN]                = 2;
		buttonMapping[BUTTON_LEFTTRIGGER_TOP]     = 4;
		buttonMapping[BUTTON_LEFTTRIGGER_BOTTOM]  = 6;
		buttonMapping[BUTTON_RIGHTTRIGGER_TOP]    = 5;
		buttonMapping[BUTTON_RIGHTTRIGGER_BOTTOM] = 7;
		buttonMapping[BUTTON_BACK]                = 8;
		buttonMapping[BUTTON_START]               = 9;
		buttonMapping[BUTTON_LEFTJOYSTICK]        = 10;
		buttonMapping[BUTTON_RIGHTJOYSTICK]       = 11;
	}
}
