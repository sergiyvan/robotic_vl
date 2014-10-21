#ifndef MOTIONEXECUTOR_H__
#define MOTIONEXECUTOR_H__

#include "ModuleFramework/Module.h"
#include "ModuleFramework/ModuleCreator.h"
#include "ModuleFramework/ModuleManager.h"

//#include "driving/threeWheel.h"
//#include "fallingDownMotion.h"
#include "walking/reinforcementLearning/learningWalker.h"
//#include "static/motion_static.h"

#include "representations/motion/activeMotion.h"
#include "representations/motion/motionRequest.h"
#include "representations/motion/motionStatus.h"
#include "representations/motion/supportFoot.h"
#include "representations/statistics/motionFramerate.h"

#include "platform/system/events.h"

BEGIN_DECLARE_MODULE(MotionExecutor)
	REQUIRE(MotionRequest)
//	REQUIRE(RobotPosture)
	REQUIRE(MotionFramerate)

	PROVIDE(MotionStatus)
	PROVIDE(ActiveMotion)
	PROVIDE(SupportFoot)
END_DECLARE_MODULE(MotionExecutor)


class MotionExecutor : public MotionExecutorBase, public ModuleManager, public EventCallback {
public:
	MotionExecutor();
	virtual ~MotionExecutor();

	/** name of this module manager */
	virtual const char* getName() const override {
		return "MotionExecutor";
	}

	virtual void init() override;
	virtual void startManager(int level) override {}
	virtual void stopManager() override {}
	virtual void execute() override;
	virtual void eventCallback(EventType eventType, void* data) override;


private:
	void standup();

//	ModuleCreator<ThreeWheel>          *threeWheelModule;
	ModuleCreator<LearningWalker>      *walkerModule;
//	ModuleCreator<MotionStatic>        *staticMotionModule;
//	ModuleCreator<FallingDownMotion>   *fallingDownModule;

	enum {
		INVALID        = 0,
		WALKER         = 1,
		THREEWHEEL     = 2
	} locomotionType;
};


#endif

