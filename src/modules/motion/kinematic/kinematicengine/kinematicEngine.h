/*
 * kinematicEngine.h
 *
 *  Created on: 09.02.2014
 *      Author: lutz
 */

#ifndef KINEMATICENGINE_H_
#define KINEMATICENGINE_H_

#include "ModuleFramework/Module.h"
#include "representations/hardware/gyroData.h"
#include "representations/motion/kinematicTree.h"
#include "representations/motion/motorPositionRequest.h"
#include "representations/motion/kinematicengine/kinematicEngineTasks.h"
#include "representations/hardware/motorAngles.h"
#include "platform/system/events.h"

#include <tools/kinematicEngine/inverseKinematicJacobian.h>

#include <random>


BEGIN_DECLARE_MODULE(KinematicEngine)
	REQUIRE(GyroData)
	REQUIRE(KinematicEngineTasks)
	REQUIRE(KinematicTree)
	REQUIRE(MotorAngles)

	PROVIDE(MotorPositionRequest)
END_DECLARE_MODULE(KinematicEngine)

class KinematicEngine : public KinematicEngineBase , EventCallback {
public:
	KinematicEngine();
	virtual ~KinematicEngine();


	virtual void init();
	virtual void execute();

	virtual void eventCallback(EventType eventType, void* data);

private:
	InverseKinematicJacobian m_inverseKinematic;

	std::uniform_real_distribution<double> distribution;

	std::default_random_engine generator;
};

#endif /* KINEMATICENGINE_H_ */
