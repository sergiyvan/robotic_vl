/*
 * kinematicEngineTaskCOMWholeRobot.h
 *
 *  Created on: 28.02.2014
 *      Author: lutz
 */

#ifndef KINEMATICENGINETASKCOMWHOLEROBOT_H_
#define KINEMATICENGINETASKCOMWHOLEROBOT_H_

#include "kinematicEngineTask.h"

class KinematicEngineTaskCOMWholeRobot : public KinematicEngineTask {
public:

	enum {
		DIMENSION_X = 1,
		DIMENSION_Y = 2,
		DIMENSION_Z = 4,
		DIMENSIONS_XY = 3,
		DIMENSIONS_XZ = 5,
		DIMENSIONS_YZ = 6,
		DIMENSIONS_XYZ = 7,
		DIMENSIONS_ALL = 7,
	};

	typedef int SubDimension;

	KinematicEngineTaskCOMWholeRobot();

	KinematicEngineTaskCOMWholeRobot(std::string name, MotorID base, KinematicTree const &tree, SubDimension subDim = DIMENSIONS_XYZ);

	virtual ~KinematicEngineTaskCOMWholeRobot();

	virtual arma::colvec getError(const KinematicTree &kinematicTree) const;
	virtual arma::colvec getTargetWithRespectToSubdims(const KinematicTree &kinematicTree) const;

	/**
	 * get the jacobian for this task
	 * @param kinematicTree
	 * @return
	 */
	virtual arma::mat getJacobianForTask(const KinematicTree &kinematicTree, bool normJacobian = false) const;

	virtual uint32_t getDimensionCnt() const {
		uint32_t ret = 0;
		if (false != m_hasTarget)
		{
			ret = m_dimCnt;
		}
		return ret;
	}

private:

	arma::mat getJacobianForTaskSub(const KinematicTree &kinematicTree, const KinematicNode *node, const KinematicNode *prevNode, KinematicMass &o_mass, bool commingFromParent) const;

	SubDimension m_subDimension;

	uint m_dimCnt;
};

#endif /* KINEMATICENGINETASKCOMWHOLEROBOT_H_ */
