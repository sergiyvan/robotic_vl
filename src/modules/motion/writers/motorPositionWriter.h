#ifndef MOTORPOSITIONWRITER_H
#define MOTORPOSITIONWRITER_H

#include "ModuleFramework/Module.h"

#include "representations/motion/motorPositionRequest.h"
#include "representations/hardware/motorAngles.h"

#include "representations/hardware/hardware.h"

BEGIN_DECLARE_MODULE(MotorPositionWriter)
	REQUIRE(MotorPositionRequest)
	REQUIRE(MotorAngles)
	PROVIDE(Hardware)
END_DECLARE_MODULE(MotorPositionWriter)


/*------------------------------------------------------------------------------------------------*/

class MotorPositionWriter
	: public MotorPositionWriterBase
{
public:
	MotorPositionWriter();
	virtual ~MotorPositionWriter();

	virtual void init() override;
	virtual void execute() override;
};

#endif

