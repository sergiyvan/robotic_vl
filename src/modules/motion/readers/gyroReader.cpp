#include "gyroReader.h"

#include "debug.h"
#include "management/config/config.h"
#include "modules/motion/motion.h"
#include "platform/hardware/imu/imu.h"
#include "services.h"

/*------------------------------------------------------------------------------------------------*/

REGISTER_MODULE(Motion, GyroReader, true, "Read the gyro values");

/*------------------------------------------------------------------------------------------------*/

REGISTER_DEBUG("hardware.gyro", PLOTTER, BASIC);

/*------------------------------------------------------------------------------------------------*/

static const std::string CfgIgnoreYaw("motorboard.imu.ignoreyaw");

namespace {
	auto cfgIgnoreYaw = ConfigRegistry::registerOption<bool>(CfgIgnoreYaw, true, "If Yaw value of gyroscope should be ignored");
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Constructor
 */

GyroReader::GyroReader() {
}

/*------------------------------------------------------------------------------------------------*/

/**
 ** Destructor
 */

GyroReader::~GyroReader() {
	services.getEvents().unregisterForEvent(EVT_CONFIGURATION_LOADED, this);
}

/*------------------------------------------------------------------------------------------------*/

/** Initialize the module. This is called by the framework when execute() is
 ** called for the first time.
 */

void GyroReader::init() {
	services.getEvents().registerForEvent(EVT_CONFIGURATION_LOADED, this);
	reloadParameter();
}

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void GyroReader::execute() {
	if (reloadParameterFlag) {
		reloadParameter();
	}

	auto q = getHardware().getIMU()->getOrientation();

	arma::mat33 rotMat;
	//roll, pitch and yaw
	std::array<Degree, 3> angles;

	float sqw = q[0]*q[0];
	float sqx = q[3]*q[3];
	float sqy = q[1]*q[1];
	float sqz = q[2]*q[2];


	angles[0] = Degree( atan2f(-2.0f * (q[3]*q[1] - q[2]*q[0]), ( sqx - sqy - sqz + sqw)) * radians);
	angles[1] = Degree(-asinf( -2.0f * (q[3]*q[2] + q[1]*q[0]) /( sqx + sqy + sqz + sqw)) * radians);
	angles[2] = Degree( atan2f( 2.0f * (q[1]*q[2] + q[3]*q[0]), (-sqx - sqy + sqz + sqw)) * radians);


	if (ignoreYaw) {

		arma::mat33 gyroPitchRotationMatrix = arma::eye(3, 3);
		arma::mat33 gyroRollRotationMatrix  = arma::eye(3, 3);

		const double cGyroPitch = cos(angles[0]);
		const double sGyroPitch = sin(angles[0]);
		const double cGyroRoll = cos(angles[1]);
		const double sGyroRoll = sin(angles[1]);

		gyroPitchRotationMatrix(0, 0) =  cGyroPitch;
		gyroPitchRotationMatrix(0, 2) =  sGyroPitch;
		gyroPitchRotationMatrix(2, 0) = -sGyroPitch;
		gyroPitchRotationMatrix(2, 2) =  cGyroPitch;

		gyroRollRotationMatrix(1, 1) =  cGyroRoll;
		gyroRollRotationMatrix(1, 2) = -sGyroRoll;
		gyroRollRotationMatrix(2, 1) =  sGyroRoll;
		gyroRollRotationMatrix(2, 2) =  cGyroRoll;

		rotMat = gyroRollRotationMatrix * gyroPitchRotationMatrix;
	} else {
		rotMat(0, 0) = 1-2*(q[2]*q[2] + q[3]*q[3]);
		rotMat(1, 1) = 1-2*(q[1]*q[1] + q[3]*q[3]);
		rotMat(2, 2) = 1-2*(q[1]*q[1] + q[2]*q[2]);

		rotMat(0, 1) = -2*q[0]*q[3] + 2*q[1]*q[2];
		rotMat(0, 2) =  2*q[0]*q[2] + 2*q[1]*q[3];
		rotMat(1, 0) =  2*q[0]*q[3] + 2*q[1]*q[2];
		rotMat(1, 2) = -2*q[0]*q[1] + 2*q[2]*q[3];
		rotMat(2, 0) = -2*q[0]*q[2] + 2*q[1]*q[3];
		rotMat(2, 1) =  2*q[0]*q[1] + 2*q[2]*q[3];
	}

	arma::colvec4 quat = arma::colvec({ q[0], q[1], q[2], q[3] });
	GyroData gyroData(getHardware().getIMU()->getTime(), angles, rotMat, quat);

	getGyroDataHistory().addGyroValue(gyroData);
	getGyroData() = gyroData;

	DEBUG_PLOTTER("hardware.gyro", "Pitch", gyroData.getTimestamp().value(), gyroData.getPitch().value());
	DEBUG_PLOTTER("hardware.gyro", "Roll",  gyroData.getTimestamp().value(), gyroData.getRoll().value());
	DEBUG_PLOTTER("hardware.gyro", "Yaw",   gyroData.getTimestamp().value(), gyroData.getYaw().value());
}

/*------------------------------------------------------------------------------------------------*/

void GyroReader::eventCallback(EventType eventType, void* data) {
	if (eventType == EVT_CONFIGURATION_LOADED) {
		reloadParameterFlag = true;
	}
}

/*------------------------------------------------------------------------------------------------*/

void GyroReader::reloadParameter() {

	ignoreYaw = cfgIgnoreYaw->get();
	reloadParameterFlag = false;
}

/*------------------------------------------------------------------------------------------------*/
