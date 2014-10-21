/*
 * kinematicEngineTaskLinearMomentum.h
 *
 *  Created on: 08.08.2014
 *      Author: lutz
 */

#ifndef KINEMATICENGINETASKLINEARMOMENTUM_H_
#define KINEMATICENGINETASKLINEARMOMENTUM_H_

#include "kinematicEngineTask.h"

class KinematicEngineTaskLinearMomentum : public KinematicEngineDynamicTask {
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

	KinematicEngineTaskLinearMomentum();
	KinematicEngineTaskLinearMomentum(std::string name, MotorID base, KinematicTree const &tree, SubDimension subDim = DIMENSIONS_XYZ);

	virtual ~KinematicEngineTaskLinearMomentum();


	virtual arma::mat getJacobianForTask(const KinematicTree &kinematicTree, bool normalizeJacobian = false) const;

	virtual arma::colvec getError(const KinematicTree &kinematicTree) const;
	virtual arma::colvec getErrorGravity(const KinematicTree &kinematicTree) const {
		arma::colvec ret = arma::zeros(m_dimCnt);
		uint8_t rowCounter = 0;
		for (uint i = 0; i < m_target.n_rows; ++i)
		{
			if (0 != (m_subDimension & (1 << i)))
			{
				ret.row(rowCounter) = m_target.row(i);
				++rowCounter;
			}
		}
		return ret;
	}

	virtual uint32_t getDimensionCnt() const {
		uint32_t ret = 0;
		if (false != m_hasTarget)
		{
			ret = m_dimCnt;
		}
		return ret;
	}

private:

	arma::mat getJacobianForTaskSub(const KinematicTree &kinematicTree,
			const KinematicNode *node,
			bool traversingUp,
			KinematicMass &o_massFromSubTree) const;

	SubDimension m_subDimension;

	uint m_dimCnt;

};

#endif /* KINEMATICENGINETASKLINEARMOMENTUM_H_ */
