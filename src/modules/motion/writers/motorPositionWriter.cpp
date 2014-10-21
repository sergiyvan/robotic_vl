/** @file
 ** This module provides writing of motor positions.
 **
 */

#include "motorPositionWriter.h"
#include "modules/motion/motion.h"
#include "platform/hardware/actuators/actuators.h"

#include "services.h"
#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

REGISTER_MODULE(Motion, MotorPositionWriter, true, "Writes motor values")

static std::string motionMotorValuesName = "motion.motorpositionswriter.motorvaluesout";
static std::string motionMotorSpeedName = "motion.motorpositionswriter.motorspeedsout";
REGISTER_DEBUG(motionMotorValuesName, TABLE, BASIC);
REGISTER_DEBUG(motionMotorSpeedName, TABLE, BASIC);

/*------------------------------------------------------------------------------------------------*/

MotorPositionWriter::MotorPositionWriter() {
}

/*------------------------------------------------------------------------------------------------*/

MotorPositionWriter::~MotorPositionWriter() {
}

/*------------------------------------------------------------------------------------------------*/

void MotorPositionWriter::init() {
}

/*------------------------------------------------------------------------------------------------*/


void MotorPositionWriter::execute() {
	auto positions = getMotorPositionRequest().getPositionRequests();
	auto offsets   = getMotorPositionRequest().getOffsetRequests();
	auto speeds    = getMotorPositionRequest().getSpeedRequests();
	auto torques   = getMotorPositionRequest().getTorqueRequests();

	getHardware().getActuators()->setOffsets(offsets);
	getHardware().getActuators()->setPositionsAndSpeeds(positions, speeds);
	getHardware().getActuators()->setTorqueEnabled(torques);


	RobotDescription const& robotDescription =*(services.getRobotModel().getRobotDescription());
	for (auto id : robotDescription.getMotorIDs()) {
		DEBUG_TABLE(motionMotorValuesName, robotDescription.getMotorName(id) , Degree(getMotorPositionRequest().getPositionRequests()[id]).value());
		DEBUG_TABLE(motionMotorSpeedName, robotDescription.getMotorName(id) , getMotorPositionRequest().getSpeedRequests()[id].value());
	}

	// hack to stay in a clean module structure
	// This will reset the written motors, to detect double writting of a motor
	const_cast<MotorPositionRequest&>(getMotorPositionRequest()).clear();
}

