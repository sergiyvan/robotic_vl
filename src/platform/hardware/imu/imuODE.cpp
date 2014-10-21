/*
 * imuODE.cpp
 *
 *  Created on: 24.09.2014
 *      Author: lutz
 */

#include <platform/hardware/imu/imuODE.h>
#include <tools/kinematicEngine/kinematicNode.h>

#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

IMU_ODE::IMU_ODE(RobotModel *model, PhysicsEnvironment *environment, KinematicTree *tree)
	: m_environment(environment)
{
	m_GyroID = model->getRobotDescription()->getEffectorID("gyroscope");
	if (m_GyroID != MOTOR_NONE) {
		const KinematicNode *gyroNode = tree->getNode(m_GyroID);
		mGyroBody = gyroNode->getODEBody();

		const dReal* quaternion = dBodyGetQuaternion(mGyroBody);
		for (uint i = 0; i < m_quaternion.size(); ++i) {
			m_quaternion[i] = quaternion[i];
		}
	}
	environment->addStepCallback(this);
}

IMU_ODE::~IMU_ODE() {
}


bool IMU_ODE::init()
{
	return true;
}

void IMU_ODE::simulatorCallback(Second timeDelta)
{
	m_lastUpdateTime = getCurrentTime();
	CriticalSectionLock csl(m_cs);

	if (m_GyroID != MOTOR_NONE) {
		const dReal* quaternion = dBodyGetQuaternion(mGyroBody);
		for (uint i = 0; i < m_quaternion.size(); ++i) {
			m_quaternion[i] = quaternion[i];
		}
	}
}


/* return quaternions */
std::array<double, 4> IMU_ODE::getOrientation() const
{
	CriticalSectionLock csl(m_cs);
	return m_quaternion;
}

/* return euler angles */
std::array<Degree, 3> IMU_ODE::getOrientationAsEulerAngles() const
{
	CriticalSectionLock csl(m_cs);
	return m_orientationEuler;
}

std::array<DPS, 3> IMU_ODE::getGyroscopeData() const
{
	CriticalSectionLock csl(m_cs);
	return m_gyroscopeData;
}

/* retrieve timestamp of current data */ // TODO: return as part of above functionss
Millisecond IMU_ODE::getTime() const
{
	CriticalSectionLock csl(m_cs);
	return m_lastUpdateTime;
}

