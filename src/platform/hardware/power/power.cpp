#include "ModuleFramework/Module.h"
#include "modules/motion/motion.h"
#include "management/commandLine.h"

#include "platform/hardware/power/power.h"

#include "representations/hardware/hardware.h"

BEGIN_DECLARE_MODULE(PowerOffModule)
	PROVIDE(Hardware)
END_DECLARE_MODULE(PowerOffModule)

class PowerOffModule : public PowerOffModuleBase {
private:

	typedef struct tag_motorSetupValues
	{
		uint8_t complianceMarginCW, complianceMarginCCW;
		uint8_t complianceSlopeCW, complianceSlopeCCW;
	} motorSetupValue_t;

public:
	PowerOffModule() {
	}

	virtual void init() {
	}

	virtual void execute() {
		INFO("Switching robot off");
		getHardware().getPower()->switchRobotOff();
	}
};

/*------------------------------------------------------------------------------------------------*/

class PowerOffModuleCallback : public CommandLineInterface {
public:
	virtual bool commandLineCallback(const CommandLine &cmdLine) {
		services.getModuleManagers().get<Motion>()->setModuleEnabled("PowerOffModule", true);
		services.runManagers();
		return true;
	}
};



/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion, PowerOffModule, false, "switch the whole robot off")

namespace {
	auto cmdPowerOff = CommandLine::getInstance().registerCommand<PowerOffModuleCallback>(
			"poweroff",
			"switch the robot power off",
			ModuleManagers::none()->enable<Motion>());
}
