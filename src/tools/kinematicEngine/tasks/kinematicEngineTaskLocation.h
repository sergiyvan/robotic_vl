/*
 * KinematicEngineTaskLocation.h
 *
 *  Created on: 14.02.2014
 *      Author: lutz
 */

#ifndef KINEMATICENGINETASKLOCATION_H_
#define KINEMATICENGINETASKLOCATION_H_

#include "kinematicEngineTask.h"

class KinematicEngineTaskLocation : public KinematicEngineTask {
public:

	enum  {
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

	KinematicEngineTaskLocation();

	KinematicEngineTaskLocation(std::string name, MotorID baseNode, MotorID effectorNode, const KinematicTree &tree,
			KinematicEngineTaskLocation::SubDimension subDimension = DIMENSIONS_XYZ);

	KinematicEngineTaskLocation(std::string name, MotorID baseNode, MotorID effectorNode, const KinematicTree &tree, arma::colvec3 target,
			KinematicEngineTaskLocation::SubDimension subDimension = DIMENSIONS_XYZ);

	virtual ~KinematicEngineTaskLocation();

	/**
	 * get the jacobian for this task
	 * @param kinematicTree
	 * @return
	 */
	virtual arma::mat getJacobianForTask(const KinematicTree &kinematicTree, bool normalizeJacobian = false) const;

	virtual arma::colvec getError(const KinematicTree &kinematicTree) const;


	virtual arma::colvec getTargetWithRespectToSubdims(const KinematicTree &kinematicTree) const {
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
	SubDimension m_subDimension;

	uint8_t m_dimCnt;

};

#endif /* KINEMATICENGINETASKLOCATION_H_ */
