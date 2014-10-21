/*
 * walkerController.h
 *
 *  Created on: Mar 16, 2014
 *      Author: stepuh
 */

#ifndef WALKERCONTROLLER_H_
#define WALKERCONTROLLER_H_

#include "tools/position.h"
#include "ModuleFramework/Module.h"
#include "platform/system/events.h"

#include "representations/hardware/gyroData.h"
#include "representations/motion/motionStart.h"
#include "representations/motion/walkerTargetValues.h"
#include "representations/motion/motionRequest.h"
//#include "representations/modeling/pathPlaning/APFPath.h"
//#include "representations/modeling/pathPlaning/pathCosts.h"

/*
 * IMPORTANT INFORMATION
 * ---------------------
 *
 * As can be seen in REQUIRE / PROVIDE statement:
 * Target Values are stored in WalkerTargetValues-representation
 * The actual current values that are used (as they are) in the walker program are
 * stored in the MotionRequest-representation!
 *
 * How the targetValues are translated into currentValues is computed in
 * the WalkerController-module, which contains a lot of expert knowledge
 * about the properties/benefits/imperfections of the walking performance in
 * certain situations.
 */



BEGIN_DECLARE_MODULE(WalkerController)
	REQUIRE(GyroData)
	REQUIRE(WalkerTargetValues)
//	REQUIRE(APFPath)
	REQUIRE(MotionStart)

	PROVIDE(MotionRequest)
//	PROVIDE(PathCosts)
END_DECLARE_MODULE(WalkerController)



class WalkerController : public WalkerControllerBase, public EventCallback {
public:

	WalkerController();
	virtual ~WalkerController();

	virtual void init() override;
	virtual void execute() override;
	virtual void eventCallback(EventType eventType, void* data) override;

private:

	// parameters
	float weightNewConfig;
	float xSmoothFactorFromZero;
	float xSmoothFactorToZero;
	float ySmoothFactorFromZero;
	float ySmoothFactorToZero;
	float yawSmoothFactorFromZero;
	float yawSmoothFactorToZero;
	float maxGyro;
	float slowDownFactor;

	void updateTargetConfiguration();

	WalkerSpeeds smoothSpeeds(const WalkerSpeeds& targetSpeeds) const;


};

#endif /* WALKERCONTROLLER_H_ */
