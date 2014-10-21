/*
 * KinematicEngineTaskFixed.h
 *
 *  Created on: 17.06.2014
 *      Author: lutz
 */

#ifndef KINEMATICENGINETASKFIXED_H_
#define KINEMATICENGINETASKFIXED_H_

#include "kinematicEngineTask.h"

#include "kinematicEngineTaskLocation.h"
#include "kinematicEngineTaskOrientation.h"

class KinematicEngineTaskFixed: public KinematicEngineTask {
public:

	enum  {
		DIMENSION_X = 1,
		DIMENSION_Y = 2,
		DIMENSION_Z = 4,
	};
	typedef int SubDimension;

	KinematicEngineTaskFixed();
	KinematicEngineTaskFixed(std::string name,
							MotorID baseNode,
							MotorID effectorNode,
							const KinematicTree &tree);

	virtual ~KinematicEngineTaskFixed();

	/**
	 * get the jacobian for this task
	 * @param kinematicTree
	 * @return
	 */
	virtual arma::mat getJacobianForTask(const KinematicTree &kinematicTree, bool normJacobian = false) const;

	virtual arma::colvec getError(const KinematicTree &kinematicTree) const;

	virtual arma::colvec getTargetWithRespectToSubdims(const KinematicTree &kinematicTree) const {
		return m_target;
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
//	SubDimension m_subDimension;
//	SubDimension m_fixedRotationAxes;

	KinematicEngineTaskLocation m_locationTask;
	KinematicEngineTaskOrientation m_rotationTaskX;
	KinematicEngineTaskOrientation m_rotationTaskY;

	uint8_t m_dimCnt;

};

#endif /* KINEMATICENGINETASKFIXED_H_ */
