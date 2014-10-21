/*
 * walkerController.cpp
 *
 *  Created on: Mar 16, 2014
 *      Author: stepuh
 */

#include "walkerController.h"
#include "utils/math/Common.h"
#include "services.h"
#include "modules/motion/motion.h"
#include "management/config/config.h"
#include <limits>



REGISTER_MODULE(Motion, WalkerController, true, "Controls walker speeds");

namespace {
	auto cfgSection = ConfigRegistry::getSection("motions.walkerController");
	auto cfg = cfgSection->registerOption<>("weightNewConfig",          0.1f, "Allowed change in robot target configuration");

	// MaxDiff Factors
	auto cfgxDiffMaxFromZero = cfgSection->registerOption<double>("xDiffMaxFromZero",    0.2f, "forward speed smoothing factor.");
	auto cfgxDiffMaxToZero = cfgSection->registerOption<double>("xDiffMaxToZero",      1.0f, "forward speed smoothing factor.");
	auto cfgyDiffMaxFromZero = cfgSection->registerOption<double>("yDiffMaxFromZero",     0.4f, "sideways speed smoothing factor.");
	auto cfgyDiffMaxToZero = cfgSection->registerOption<double>("yDiffMaxToZero",      1.0f, "sideways speed smoothing factor.");
	auto cfgyawDiffFromZero = cfgSection->registerOption<double>("yawDiffFromZero",     .5f, "rotation (yaw) speed smoothing factor.");
	auto cfgyawDiffToZero = cfgSection->registerOption<double>("yawDiffToZero",       1.0f, "rotation (yaw) speed smoothing factor.");

	auto cfgmaxGyro = cfgSection->registerOption<double>("maxGyro", 6.f, "if gyro is greater than this -> slowing down");
	auto cfgslowDownFactor = cfgSection->registerOption<double>("slowDownFactor", 0.01f, "if gyro is greater than this -> slowing down");

}


WalkerController::WalkerController()
	: weightNewConfig(0.1)
	, xSmoothFactorFromZero(0.0)
	, xSmoothFactorToZero(0.0)
	, ySmoothFactorFromZero(0.0)
	, ySmoothFactorToZero(0.0)
	, yawSmoothFactorFromZero(0.0)
	, yawSmoothFactorToZero(0.0)
{
}



WalkerController::~WalkerController()
{
	services.getEvents().unregisterForEvent(EVT_CONFIGURATION_LOADED, this);
}



void WalkerController::init()
{
	// Register for configuration changes
	services.getEvents().registerForEvent(EVT_CONFIGURATION_LOADED, this);
	eventCallback(EVT_CONFIGURATION_LOADED, &services.getConfig());

}



void WalkerController::execute()
{
	WalkerSpeeds speeds = {0., 0., 0.};
//	getPathCosts() = std::numeric_limits<double>::max();
	switch (getWalkerTargetValues().getLastInstructionMethod()) {
		case InstructionMethod::VIA_SPEEDS:
			speeds = smoothSpeeds(getWalkerTargetValues().getTargetSpeeds());
			break;
		default:
			break;
	}
	getMotionRequest().walkerParam.setSpeeds(speeds.x, speeds.y, speeds.yaw);
}



void WalkerController::updateTargetConfiguration()
{
	// weighted running average
	WalkerConfiguration averageConfiguration;
	averageConfiguration.x   = getWalkerTargetValues().getTargetConfiguration().x   * weightNewConfig + getMotionRequest().walkerParam.getCurrentTargetConfiguration().x   * (1.0 - weightNewConfig);
	averageConfiguration.y   = getWalkerTargetValues().getTargetConfiguration().y   * weightNewConfig + getMotionRequest().walkerParam.getCurrentTargetConfiguration().y   * (1.0 - weightNewConfig);
	averageConfiguration.yaw = getWalkerTargetValues().getTargetConfiguration().yaw * weightNewConfig + getMotionRequest().walkerParam.getCurrentTargetConfiguration().yaw * (1.0 - weightNewConfig);
	getMotionRequest().walkerParam.setCurrentTargetConfiguration(averageConfiguration);
}



WalkerSpeeds WalkerController::smoothSpeeds(const WalkerSpeeds& targetSpeeds) const
{
	float targetX   = 0.0;
	float targetY   = 0.0;
	float targetYaw = 0.0;

	if (std::abs(getGyroData().getRoll().value()) < maxGyro) {

		double fraction =  std::abs(getGyroData().getRoll().value()) * slowDownFactor;
		fraction = Math::limited(1 - fraction, 0.0, 1.0);

		targetX   = targetSpeeds.x * fraction;
		targetY   = targetSpeeds.y * fraction;
		targetYaw = targetSpeeds.yaw * fraction;
	}

	float diffX   = targetX   - getMotionRequest().walkerParam.getForwardSpeed();
	float diffY   = targetY   - getMotionRequest().walkerParam.getSidewardSpeed();
	float diffYaw = targetYaw - getMotionRequest().walkerParam.getRotationSpeed();


	if (getMotionRequest().walkerParam.getForwardSpeed() > 0) {
		diffX = Math::limited(diffX, float(-xSmoothFactorToZero), float(xSmoothFactorFromZero));
	} else {
		diffX = Math::limited(diffX, float(-xSmoothFactorFromZero), float(xSmoothFactorToZero));
	}

	if (getMotionRequest().walkerParam.getSidewardSpeed() > 0) {
		diffY = Math::limited(diffY, float(-ySmoothFactorToZero), float(ySmoothFactorFromZero));
	} else {
		diffY = Math::limited(diffY, float(-ySmoothFactorFromZero), float(ySmoothFactorToZero));
	}

	if (getMotionRequest().walkerParam.getRotationSpeed() > 0) {
		diffYaw = Math::limited(diffYaw, float(-yawSmoothFactorToZero), float(yawSmoothFactorFromZero));
	} else {
		diffYaw = Math::limited(diffYaw, float(-yawSmoothFactorFromZero), float(yawSmoothFactorToZero));
	}

	WalkerSpeeds smoothedSpeeds;
	smoothedSpeeds.x   = getMotionRequest().walkerParam.getForwardSpeed()  + diffX;
	smoothedSpeeds.y   = getMotionRequest().walkerParam.getSidewardSpeed() + diffY;
	smoothedSpeeds.yaw = getMotionRequest().walkerParam.getRotationSpeed() + diffYaw;
	return smoothedSpeeds;
}


void WalkerController::eventCallback(EventType eventType, void* data) {
	if (eventType == EVT_CONFIGURATION_LOADED) {
		xSmoothFactorFromZero     = cfgxDiffMaxFromZero->get();
		xSmoothFactorToZero       = cfgxDiffMaxToZero->get();
		ySmoothFactorFromZero     = cfgyDiffMaxFromZero->get();
		ySmoothFactorToZero       = cfgyDiffMaxToZero->get();
		yawSmoothFactorFromZero   = cfgyawDiffFromZero->get();
		yawSmoothFactorToZero     = cfgyawDiffToZero->get();
		maxGyro                   = cfgmaxGyro->get();
		slowDownFactor            = cfgslowDownFactor->get();
	}
}

