/*
 * actuatorsODE.cpp
 *
 *  Created on: 24.09.2014
 *      Author: lutz
 */

#include "actuatorsODE.h"

#include <tools/kinematicEngine/physics/ODEMotor.h>

ActuatorsODE::ActuatorsODE(PhysicsEnvironment *environment, KinematicTree *tree)
	: m_physicsEnvironment(environment)
	, m_tree(tree)
{
	m_physicsEnvironment->addStepCallback(this);
	CriticalSectionLock csl(m_cs);
	std::map<MotorID, ODEMotor*>& physicsMotors = m_physicsEnvironment->getMotors();

	for (std::pair<MotorID, ODEMotor*> const& motor: physicsMotors)
	{
		motorData[motor.first].position = motor.second->getCurAngle();
		motorData[motor.first].speed    = motor.second->getCurSpeed();
	}
}

ActuatorsODE::~ActuatorsODE() {
}


bool ActuatorsODE::init()  {
	return true;
}


void ActuatorsODE::simulatorCallback(Second timeDelta)
{
	CriticalSectionLock csl(m_cs);
	std::map<MotorID, ODEMotor*>& physicsMotors = m_physicsEnvironment->getMotors();

	for (std::pair<MotorID, ODEMotor*> const& motor: physicsMotors)
	{
		motorData[motor.first].position = motor.second->getCurAngle();
		motorData[motor.first].speed    = motor.second->getCurSpeed();
	}

	for (std::pair<MotorID, Degree> motor : mTargetAngles)
	{
		std::map<MotorID, ODEMotor*>::iterator iter = physicsMotors.find(motor.first);
		if (iter != physicsMotors.end())
		{
			iter->second->setDesiredAngle(motor.second);
		}
	}

	for (std::pair<MotorID, RPM> motor : mTargetSpeeds)
	{
		std::map<MotorID, ODEMotor*>::iterator iter = physicsMotors.find(motor.first);
		if (iter != physicsMotors.end())
		{
			iter->second->setDesiredSpeed(motor.second);
		}
	}

	for (std::pair<MotorID, bool> motor : mTorqueEnabled)
	{
		std::map<MotorID, ODEMotor*>::iterator iter = physicsMotors.find(motor.first);
		if (iter != physicsMotors.end())
		{
			iter->second->enableMotor(motor.second);
		}
	}

	for (std::pair<MotorID, ODEMotor*> const& motor: physicsMotors)
	{
		motor.second->simulatorCallback(timeDelta);
	}
}

/// get all motor positions
std::map<MotorID, Degree> ActuatorsODE::getPositions() const
{
	CriticalSectionLock csl(m_cs);
	std::map<MotorID, Degree> retMap;

	for (std::pair<MotorID, MotorData> const& motor : motorData) {
		retMap[motor.first] = motor.second.position;
	}

	return retMap;
}

/// get all (current) motor speeds
std::map<MotorID, RPM> ActuatorsODE::getSpeeds() const
{
	CriticalSectionLock csl(m_cs);
	std::map<MotorID, RPM> retMap;

	for (std::pair<MotorID, MotorData> const& motor : motorData) {
		retMap[motor.first] = motor.second.speed;
	}

	return retMap;
}

/// get all motor data (positions/speed)
std::map<MotorID, MotorData> ActuatorsODE::getMotorData() const
{
	CriticalSectionLock csl(m_cs);
	return std::map<MotorID, MotorData>(motorData);
}

/// set the positions and speeds of the servos
void ActuatorsODE::setPositionsAndSpeeds(const std::map<MotorID, Degree> &positions, const std::map<MotorID, RPM> &speeds)
{
	CriticalSectionLock csl(m_cs);

	mTargetAngles = positions;
	mTargetSpeeds = speeds;
}

/// enable/disable the torque
void ActuatorsODE::setTorqueEnabled(const std::map<MotorID, bool> &motors)
{
	CriticalSectionLock csl(m_cs);
	mTorqueEnabled = motors;
}

