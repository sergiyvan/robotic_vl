#ifndef GYRO_READER_H
#define GYRO_READER_H

#include "ModuleFramework/Module.h"

#include "representations/hardware/gyroDataHistory.h"
#include "representations/hardware/hardware.h"
#include "representations/hardware/rawMagDataHistory.h"

#include "platform/system/events.h"

/*------------------------------------------------------------------------------------------------*/

BEGIN_DECLARE_MODULE(GyroReader)
	PROVIDE(GyroDataHistory)
	PROVIDE(GyroData)
	PROVIDE(Hardware)
	PROVIDE(RawMagDataHistory)
END_DECLARE_MODULE(GyroReader)

/*------------------------------------------------------------------------------------------------*/

class GyroReader
	: public GyroReaderBase
	, public EventCallback {
public:
	GyroReader();
	virtual ~GyroReader();

	virtual void init() override;
	virtual void execute() override;

	virtual void eventCallback(EventType eventType, void* data) override;
private:
	void reloadParameter();

	bool ignoreYaw;

	bool reloadParameterFlag;
};

/*------------------------------------------------------------------------------------------------*/

#endif
