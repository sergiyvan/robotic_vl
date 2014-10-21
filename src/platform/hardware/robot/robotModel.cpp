#include "robotModel.h"

#include "services.h"
#include "debug.h"

// hardware interfaces
#include "platform/hardware/actuators/actuators.h"
#include "platform/hardware/beeper/beeper.h"
#include "platform/hardware/buttons/buttons.h"
#include "platform/hardware/imu/imu.h"
#include "platform/hardware/ledsignals/ledSignals.h"
#include "platform/hardware/power/power.h"
#include "platform/hardware/clock/clock.h"

#include "platform/hardware/motions/motions.h"

// robot description
#include "platform/hardware/robot/robotDescription.h"

#include "management/config/config.h"
#include "management/config/configRegistry.h"

#include "utils/utils.h"

#include <string>


/*------------------------------------------------------------------------------------------------*/

#if defined DESKTOP
#define DEFAULTROBOTMODEL "ode"
#define ROBOTDESCRIPTION  "robotDescription.xml"
#endif


/*------------------------------------------------------------------------------------------------*/

REGISTER_ROBOTMODEL("generic", RobotModel, "Generic RobotModel");

namespace {
	auto cfgModel       = ConfigRegistry::registerOption<std::string>("robot.model",       DEFAULTROBOTMODEL,      "Robot model");
	auto cfgDescription = ConfigRegistry::registerOption<std::string>("robot.description", ROBOTDESCRIPTION,       "Robot description file");
}

/*------------------------------------------------------------------------------------------------*/

RobotModel::RobotModel()
	: hardwareIsInitialized(false)
	, actuators(new Actuators())
	, beeper(new Beeper())
	, buttons(new Buttons())
	, imu(new IMU())
	, ledSignals(new LEDSignals())
	, motions(new Motions())
	, power(new Power())
	, clock(new Clock())
{
	std::string robotDescriptionPath = cfgDescription->get();
	robotDescription = std::unique_ptr<RobotDescription>( new RobotDescription(robotDescriptionPath) );
}


/*------------------------------------------------------------------------------------------------*/

RobotModel::~RobotModel() {
}


/*------------------------------------------------------------------------------------------------*/

RobotModel* RobotModel::createRobotModel() {
	std::string modelName = cfgModel->get();

	if (Factory<RobotModel>::has(modelName)) {
		INFO("Creating robot model \"%s\" (%s)", modelName.c_str(), Factory<RobotModel>::getDescription(modelName).c_str());
		return Factory<RobotModel>::getNew(modelName);
	}

	ERROR("Unknown robot model \"%s\":", modelName.c_str());
	INFO("Available RobotModels:");
	for (auto x : Factory<RobotModel>::getAvailableObjectNames()) {
		INFO(" %s", x.c_str());
	}
	INFO("=============");
	return new RobotModel();
}


/*------------------------------------------------------------------------------------------------*/

bool RobotModel::init() {
	// initialize hardware subsystems
	actuators->init();
	beeper->init();
	buttons->init();
	imu->init();
	ledSignals->init();
	motions->init();
	power->init();

	return true;
}


/*------------------------------------------------------------------------------------------------*/

std::map<MotorID, Degree> RobotModel::getIdleValues() const {
	if (false == motions->hasMotion(MOTION_STAND_IDLE))
		return {};

	std::map<MotorID, Degree> retValues;

	de::fumanoids::message::RobotMotion* robotMotion = motions->getMotion(MOTION_STAND_IDLE);
	if (nullptr == robotMotion) {
		ERROR("Could not get idle motion values as there is no such motion defined.");
		return {};
	}
	// get motor values
	for (int i = 0; i < robotMotion->motion_size(); ++i) {
		de::fumanoids::message::MotorMotion motorMotion = robotMotion->motion(i);

		int last = motorMotion.move_size() - 1;
		const auto &pos = motorMotion.move(last).position();
		if (pos.has_oldvalue()) {
			// old motion, uses the RX-servo configuration (0° = 511.5, range 0..1023)
			retValues[motorMotion.motorid()] = ( (pos.oldvalue()-511.5)*300.0/1023.0 )*degrees;
		} else {
			retValues[motorMotion.motorid()] = pos.angle()*degrees;
		}
	}

	return retValues;
}
