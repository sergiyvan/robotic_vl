#include "ModuleFramework/Module.h"

#include "modules/motion/motion.h"

#include "platform/hardware/clock/clock.h"

#include "representations/hardware/motionTime.h"
#include "representations/hardware/hardware.h"

BEGIN_DECLARE_MODULE(MotionTimeProvider)
	RECYCLE(Hardware)

	PROVIDE(MotionTime)
END_DECLARE_MODULE(MotionTimeProvider)

class MotionTimeProvider : public MotionTimeProviderBase {
public:
	MotionTimeProvider() {}
	virtual ~MotionTimeProvider() {}

	virtual void init() {}
	virtual void execute() {
		getMotionTime().setCurTime(getHardware().getClock()->getCurrentTime());
	}
};


REGISTER_MODULE(Motion, MotionTimeProvider, true, "provide the current time of the robot");
