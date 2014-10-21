#include "ModuleFramework/Module.h"

#include "modules/motion/motion.h"

#include "platform/hardware/power/power.h"

#include "representations/hardware/batteryInfo.h"
#include "representations/hardware/hardware.h"

#include "utils/math/Math.h"

#include <list>
#include <memory>
#include <string>


BEGIN_DECLARE_MODULE(BatteryReader)
	RECYCLE(Hardware)

	PROVIDE(BatteryInfo)
END_DECLARE_MODULE(BatteryReader)

class BatteryReader : public BatteryReaderBase {
public:
	BatteryReader() {}
	virtual ~BatteryReader() {}

	virtual void init() {}
	virtual void execute() {
		getBatteryInfo().batteryPercent = getHardware().getPower()->getBatteryPercent();
	}
};


REGISTER_MODULE(Motion, BatteryReader, true, "Read the battery info");
