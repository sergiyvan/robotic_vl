#include "modules/motion/motion.h"

#include "ModuleFramework/Module.h"

#include "representations/hardware/motorAngles.h"
#include "representations/motion/kinematicTree.h"
#include "representations/hardware/gyroData.h"

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

BEGIN_DECLARE_MODULE(MotionKinematicTreeProvider)
	REQUIRE(MotorAngles)
	REQUIRE(GyroData)

	PROVIDE(KinematicTree)
	PROVIDE(IdleKinematicTree)
END_DECLARE_MODULE(MotionKinematicTreeProvider)

class MotionKinematicTreeProvider
	: public MotionKinematicTreeProviderBase
{
public:
	MotionKinematicTreeProvider() {}
	virtual ~MotionKinematicTreeProvider() {}

	virtual void init() {
		getKinematicTree().setup(*services.getRobotModel().getRobotDescription());

		getIdleKinematicTree().setup(*services.getRobotModel().getRobotDescription());
	}

	virtual void execute() {
		getKinematicTree().setMotorValues(getMotorAngles().getPositions());
		getKinematicTree().setMotorSpeeds(getMotorAngles().getSpeeds());
		getKinematicTree().setGyroscopeAngles(getGyroData().getRotMat());
	}
};


REGISTER_MODULE(Motion, MotionKinematicTreeProvider, true, "Provide current kinematic tree (motion layer)")

