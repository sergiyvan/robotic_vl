/*
 * inverseKinematicsSRJacobian.h
 *
 *  Created on: 25.06.2014
 *      Author: lutz
 */

#ifndef INVERSEKINEMATICSSRJACOBIAN_H_
#define INVERSEKINEMATICSSRJACOBIAN_H_

#include "inverseKinematicJacobian.h"

/*
 * just like the inverse kinematics with dampened least squares but more robust against singularities
 */
class InverseKinematicsSRJacobian : public InverseKinematicJacobian {
public:
	InverseKinematicsSRJacobian();
	virtual ~InverseKinematicsSRJacobian();

	virtual double iterationSteps(KinematicTree const& tree, std::map<MotorID, Degree>& o_angles, uint iterationCnt);

	void setDOGSuppressionMinLen(double dogSuppressionMinLen) {
		m_dogSuppressionMinLen = dogSuppressionMinLen;
	}

	double getDOGSuppressionMinLen() {
		return m_dogSuppressionMinLen;
	}

protected:
	double m_dogSuppressionMinLen;
};

#endif /* INVERSEKINEMATICSSRJACOBIAN_H_ */
