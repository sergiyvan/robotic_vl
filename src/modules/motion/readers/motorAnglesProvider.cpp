#include "motorAnglesProvider.h"

#include "services.h"

#include "modules/motion/motion.h"

#include "platform/hardware/actuators/actuators.h"
#include "platform/hardware/robot/robotModel.h"
#include "platform/system/timer.h"

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

REGISTER_MODULE(Motion, MotorAnglesProvider, true, "Reads Motorvalues and porvides them in motorAngles and -History");

static std::string motionMotorValuesName = "motion.motoranglesprovider.motorvaluesin";
static std::string motionMotorSpeedName = "motion.motoranglesprovider.motorspeedsin";
REGISTER_DEBUG(motionMotorValuesName, TABLE, BASIC);
REGISTER_DEBUG(motionMotorSpeedName, TABLE, BASIC);

MotorAnglesProvider::MotorAnglesProvider() {
}

MotorAnglesProvider::~MotorAnglesProvider() {
}


void MotorAnglesProvider::init() {
}


void MotorAnglesProvider::execute() {
	Millisecond timestamp = getCurrentTime();

	// Reading motor position,speed and load values
	auto data      = getHardware().getActuators()->getMotorData();

	std::map<MotorID, Degree> positions;
	std::map<MotorID, RPM>    speeds;
	for (const auto &it : data) {
		positions[it.first] = it.second.position;
		speeds[it.first]    = it.second.speed;
	}
	auto offsets   = getHardware().getActuators()->getOffsets();

	RobotDescription const& robotDescription =*(services.getRobotModel().getRobotDescription());
	for (auto id : robotDescription.getMotorIDs()) {
		DEBUG_TABLE(motionMotorValuesName, robotDescription.getMotorName(id) , Degree(positions[id]).value());
		DEBUG_TABLE(motionMotorSpeedName, robotDescription.getMotorName(id) , speeds[id].value());
	}

	getMotorAngles().setValues(timestamp, positions, offsets, speeds);
	getMotorAnglesHistory().addMotorAngles(getMotorAngles());
}
