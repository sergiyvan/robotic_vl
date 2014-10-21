/*
 * actuatorsODE.h
 *
 *  Created on: 24.09.2014
 *      Author: lutz
 */

#ifndef ACTUATORSODE_H_
#define ACTUATORSODE_H_

#include "actuators.h"
#include <tools/kinematicEngine/physics/physicsEnvironment.h>
#include <platform/system/thread.h>


class ActuatorsODE : public Actuators, PhysicsEnvironmentStepCallback {
public:
	ActuatorsODE(PhysicsEnvironment *environment, KinematicTree *tree);
	virtual ~ActuatorsODE();

	virtual bool init() override;

	/// get all motor positions
	virtual std::map<MotorID, Degree>    getPositions()   const override;

	/// get all (current) motor speeds
	virtual std::map<MotorID, RPM>       getSpeeds()      const override;

	/// get all motor data (positions/speed)
	virtual std::map<MotorID, MotorData> getMotorData()   const override;

	/// set the positions and speeds of the servos
	virtual void setPositionsAndSpeeds(const std::map<MotorID, Degree> &positions, const std::map<MotorID, RPM> &speeds) override;

	/// enable/disable the torque
	virtual void setTorqueEnabled(const std::map<MotorID, bool> &motors) override;

	virtual void simulatorCallback(Second timeDelta);
private:
	PhysicsEnvironment *m_physicsEnvironment;
	KinematicTree *m_tree;

	std::map<MotorID, MotorData> motorData;

	std::map<MotorID, Degree> mTargetAngles;
	std::map<MotorID, RPM> mTargetSpeeds;
	std::map<MotorID, bool> mTorqueEnabled;

	CriticalSection m_cs;
};

#endif /* ACTUATORSODE_H_ */
