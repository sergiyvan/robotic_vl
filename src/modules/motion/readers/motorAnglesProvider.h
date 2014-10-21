#ifndef MOTORANGLES_PROVIDER_H
#define MOTORANGLES_PROVIDER_H

#include "ModuleFramework/Module.h"

#include "representations/hardware/motorAnglesHistory.h"
#include "representations/motion/motorPositionRequest.h"
#include "representations/hardware/hardware.h"

BEGIN_DECLARE_MODULE(MotorAnglesProvider)
	RECYCLE(MotorPositionRequest)
	RECYCLE(Hardware)

	PROVIDE(MotorAnglesHistory)
	PROVIDE(MotorAngles)
END_DECLARE_MODULE(MotorAnglesProvider)

class MotorAnglesProvider
	: public MotorAnglesProviderBase
{
public:
	MotorAnglesProvider();
	virtual ~MotorAnglesProvider();

	virtual void init() override;
	virtual void execute() override;
};

#endif
