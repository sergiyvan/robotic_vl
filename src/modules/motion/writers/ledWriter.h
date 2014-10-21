#ifndef LEDWRITER_H
#define LEDWRITER_H

#include "ModuleFramework/Module.h"

#include "representations/hardware/ledRequest.h"
#include "representations/hardware/hardware.h"

BEGIN_DECLARE_MODULE(LEDWriter)
	REQUIRE(LEDRequest)

	PROVIDE(Hardware)
END_DECLARE_MODULE(LEDWriter)


class LEDWriter : public LEDWriterBase {
public:
	LEDWriter();
	virtual ~LEDWriter();

	virtual void init() override;
	virtual void execute() override;

private:
	std::array<bool, Button::CT> buttonLEDStatus;
	std::map<MotorID, bool>      motorLEDStatus;

	void updateButtonLED(Button _id);

	bool currentBlinkingState(LEDMode _mode);
};

#endif

