/*
 * imuODE.h
 *
 *  Created on: 24.09.2014
 *      Author: lutz
 */

#ifndef IMUODE_H_
#define IMUODE_H_

#include "imu.h"
#include <tools/kinematicEngine/physics/physicsEnvironment.h>
#include <platform/hardware/robot/robotModel.h>
#include <platform/system/thread.h>

#include <representations/motion/kinematicTree.h>

class IMU_ODE : public IMU, PhysicsEnvironmentStepCallback {
public:
	IMU_ODE(RobotModel *model, PhysicsEnvironment *enfironment, KinematicTree *tree);
	virtual ~IMU_ODE();

	virtual bool init() override;

	/* return quaternions */
	virtual std::array<double, 4> getOrientation() const override;

	/* return euler angles */
	virtual std::array<Degree, 3> getOrientationAsEulerAngles() const override;

	virtual std::array<DPS, 3> getGyroscopeData() const override;

	/* retrieve timestamp of current data */ // TODO: return as part of above functionss
	virtual Millisecond getTime() const override;

	virtual void simulatorCallback(Second timeDelta) override;

private:
	PhysicsEnvironment *m_environment;
	CriticalSection m_cs;

	std::array<double, 4> m_quaternion;
	std::array<Degree, 3> m_orientationEuler;
	std::array<DPS, 3>    m_gyroscopeData;
	Millisecond m_lastUpdateTime;

	dBodyID mGyroBody;
	MotorID m_GyroID;
};

#endif /* IMUODE_H_ */
