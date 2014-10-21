#include "statusOutput.h"
#include "services.h"
#include "platform/system/timer.h"
#include "management/config/config.h"

#include "communication/comm.h"

#include <msg_status.pb.h>


namespace {
	#define DEFAULTSTATUSINTERVAL      (500*milliseconds)
	auto cfgSendInterval = ConfigRegistry::getInstance().registerOption<Millisecond>("io.status.sendinterval", DEFAULTSTATUSINTERVAL, "How often (every X ms) to send our generic status packet");
}

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

StatusOutput::StatusOutput()
	: hasRobotStatusInfo(false)
	, lastStatusTimestamp(0)
{
	cs.setName("StatusOutput::cs");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

StatusOutput::~StatusOutput() {
	if (isRunning()) {
		cancel(true);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

bool StatusOutput::init() {
	run();

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Thread main of StatusOutput. Regularly (or if requested) sends out status
 ** information to the world.
 */

void StatusOutput::threadMain() {

	while (isRunning()) {
		if (hasRobotStatusInfo) {
			CriticalSectionLock lock(cs);

			lastStatusTimestamp = getCurrentTime();
			services.getComm().broadcastMessage(cachedRobotStatus);
			hasRobotStatusInfo = false;
		}
		else if (getCurrentTime() - lastStatusTimestamp > cfgSendInterval->get()) {
			lastStatusTimestamp = getCurrentTime();

			de::fumanoids::message::Message msg;
			de::fumanoids::message::Status &status = *msg.MutableExtension(de::fumanoids::message::status);

			// add team info
			status.set_timestamp(lastStatusTimestamp.value());
			status.set_timeonline((lastStatusTimestamp - services.getStarttime()).value());

			services.getComm().broadcastMessage(msg);
		}

		delay(50*milliseconds);
	}
}
