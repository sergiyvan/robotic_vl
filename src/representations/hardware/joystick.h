#ifndef JOYSTICK_H__

#include <string>
#include <map>

#include <inttypes.h>


typedef enum {
	  BUTTON_LEFT                = 0
	, BUTTON_UP                  = 1
	, BUTTON_RIGHT               = 2
	, BUTTON_DOWN                = 3
	, BUTTON_LEFTTRIGGER_TOP     = 4
	, BUTTON_LEFTTRIGGER_BOTTOM  = 5
	, BUTTON_RIGHTTRIGGER_TOP    = 6
	, BUTTON_RIGHTTRIGGER_BOTTOM = 7
	, BUTTON_BACK                = 8
	, BUTTON_START               = 9
	, BUTTON_DPAD_LEFT           = 10
	, BUTTON_DPAD_UP             = 11
	, BUTTON_DPAD_RIGHT          = 12
	, BUTTON_DPAD_DOWN           = 13
	, BUTTON_LEFTJOYSTICK        = 14
	, BUTTON_RIGHTJOYSTICK       = 15
} GamepadButton;

typedef enum {
	  LEFTHAND_HORIZONTAL        = 0
	, LEFTHAND_VERTICAL          = 1
	, RIGHTHAND_HORIZONTAL       = 2
	, RIGHTHAND_VERTICAL         = 3
	, LEFTTRIGGER                = 4
	, RIGHTTRIGGER               = 5
} GamepadAxis;


/*------------------------------------------------------------------------------------------------*/

class Joystick {
public:
	Joystick()
		: active(false)
		, modelName("UNKNOWN")
		, axisCount(0)
		, buttonCount(0)
		, buttonData(NULL)
		, buttonTriggerData(NULL)
		, axisData(NULL)
	{
	}

	virtual ~Joystick();

	/// whether a joystick or gamepad is connected
	bool active;


	// set model and mappings
	void setModel(const std::string &modelName, uint8_t axisCount, uint8_t buttonCount);


	/** get name of model */
	const std::string& getModel() {
		return modelName;
	}


	/** Set the value of an axis.
	 **
	 ** @param axisID  ID of axis
	 ** @param value   value
	 */

	void setAxis(uint8_t axisID, int32_t value) {
		axisData[axisID] = value;
	}


	/** Set the state of a button.
	 **
	 ** @param buttonID  ID of button
	 ** @param pressed   Whether button is currently pressed down
	 */
	void setButton(uint8_t buttonID, bool pressed) {
		if (buttonData[buttonID] != pressed)
			buttonTriggerData[buttonID] = pressed;

		buttonData[buttonID] = (pressed != 0);
	}


	/** Checks whether button was pressed. Will only respond with 'true'
	 ** once per button-down.
	 **
	 ** @param buttonID
	 ** @return true if button was pressed
	 */

	bool wasPressed(GamepadButton buttonID) const {
		auto it = buttonMapping.find(buttonID);
		if (it == buttonMapping.end())
			return false;

		uint8_t id = it->second;
		bool pressed = (buttonTriggerData[id] != 0);
		if (pressed)
			buttonTriggerData[id] = false;
		return pressed;
	}


	/** Retrieves current state of button.
	 **
	 ** @param buttonID
	 ** @return whether button is currently pressed down.
	 */
	bool isCurrentStatePressed(GamepadButton buttonID) const {
		auto it = buttonMapping.find(buttonID);
		if (it == buttonMapping.end())
			return false;

		uint8_t id = it->second;
		return buttonData[id] != 0;
	}


	/** Retrieve current value for an axis.
	 **
	 ** @param axis       Axis to query
	 ** @param threshold  Values in [-threshold, +threshold] will be mapped to 0
	 ** @return current value
	 */

	int16_t getAxisValue(GamepadAxis axis, int threshold=0) const {
		auto it = axisMapping.find(axis);
		if (it != axisMapping.end()) {
			int index = it->second;
			int16_t value = axisData[index-1];
			if (index < 0)
				value = -value;

			if (abs(value) < threshold)
				return 0;

			return value;
		} else
			return 0;
	}

protected:

	/// name of the connected joystick or gamepad
	std::string modelName;

	/// number of axes the joystick/gamepad has
	int32_t axisCount;

	/// number of buttons the joystick/gamepad has
	int32_t buttonCount;

	/// the current button values
	char *buttonData;

	/// the current button values
	char *buttonTriggerData;

	/// the current axis values
	int32_t *axisData;

	/// the button mapping
	std::map<GamepadButton, int16_t> buttonMapping;

	/// the axis mapping
	std::map<GamepadAxis, uint8_t> axisMapping;

};


#endif
