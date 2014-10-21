/*
 * KinematicEngineTaskLocation.cpp
 *
 *  Created on: 14.02.2014
 *      Author: lutz
 */

#include <tools/kinematicEngine/tasks/kinematicEngineTaskLocation.h>
#include <armadillo>

KinematicEngineTaskLocation::KinematicEngineTaskLocation() : KinematicEngineTask(),
	m_subDimension(DIMENSION_X | DIMENSION_Y | DIMENSION_Z),
	m_dimCnt(3)
{
}

KinematicEngineTaskLocation::KinematicEngineTaskLocation(std::string name, MotorID baseNode, MotorID effectorNode, const KinematicTree &tree, KinematicEngineTaskLocation::SubDimension subDimension )
	: KinematicEngineTask(name, baseNode, effectorNode, tree),
	m_subDimension(subDimension)
{
	m_dimCnt = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (0 != (m_subDimension & (1 << i)))
		{
			++m_dimCnt;
		}
	}
}

KinematicEngineTaskLocation::KinematicEngineTaskLocation(std::string name, MotorID baseNode, MotorID effectorNode, const KinematicTree &tree, arma::colvec3 target, KinematicEngineTaskLocation::SubDimension subDimension )
	: KinematicEngineTask(name, baseNode, effectorNode, tree, target),
	m_subDimension(subDimension)
{
	m_dimCnt = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (0 != (m_subDimension & (1 << i)))
		{
			++m_dimCnt;
		}
	}
}

KinematicEngineTaskLocation::~KinematicEngineTaskLocation()
{
}

arma::mat KinematicEngineTaskLocation::getJacobianForTask(const KinematicTree &kinematicTree, bool normalizeJacobian) const
{
	arma::mat jacobian = arma::zeros(m_target.n_rows, kinematicTree.getMotorCt());
	arma::mat retJacobian = arma::zeros(m_dimCnt, kinematicTree.getMotorCt());

	if (false != m_hasTarget)
	{
		arma::mat44 forward = arma::eye(4, 4);
		arma::mat44 intermediateTransition = arma::eye(4, 4);

		for (uint32_t pathIndex = 0; pathIndex < m_invPath.size(); ++pathIndex)
		{
			const KinematicNode *node = m_invPath[pathIndex].m_node;

			arma::colvec3 vecToEndeffector = forward.col(3).rows(0, 2);

			if ((false == node->isFixedNode()) &&
				(KinematicPathNode::Direction::LINK != m_invPath[pathIndex].m_direction))
			{
				arma::colvec3 partialDerivative = node->getPartialDerivativeOfLocationToEffector(vecToEndeffector);
				if (KinematicPathNode::Direction::FROM_PARENT == m_invPath[pathIndex].m_direction)
				{
					/* in this case the rotation axis of the joint is inverted... */
					partialDerivative = -partialDerivative;
				}
				jacobian.col(kinematicTree.toInt(node->getID())) = partialDerivative;
			}

			if (pathIndex < m_invPath.size() - 1)
			{
				const KinematicNode *nextNode = m_invPath[pathIndex + 1].m_node;
				intermediateTransition = node->getInvMatrixToRelative(nextNode->getID());
				forward = intermediateTransition * forward;

				/* update the previously calculated entries in the jacobian according to our current orientation */
				jacobian = intermediateTransition.submat(0, 0, 2, 2) * jacobian;
			}
		}

		removeDOFfromJacobian(jacobian, kinematicTree);
		retJacobian = removeSubdimensions(jacobian, m_subDimension, m_dimCnt);
		if (normalizeJacobian) {
			normJacobian(retJacobian);
		}
	}


	return retJacobian;
}


arma::colvec KinematicEngineTaskLocation::getError(const KinematicTree &kinematicTree) const {
	/* calculate the position of the effector */
	const arma::mat44 transition = kinematicTree.getTransitionMatrixFromTo(m_baseNode, m_effectorNode);
	arma::colvec3 endeffector = transition.col(3).rows(0, 2);
	arma::colvec value = arma::zeros(m_dimCnt);

	uint8_t rowCounter = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (0 != (m_subDimension & (1 << i)))
		{
			value.row(rowCounter) = endeffector.row(i);
			++rowCounter;
		}
	}

	/* get target */
	arma::colvec target = arma::zeros(m_dimCnt);
	rowCounter = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (0 != (m_subDimension & (1 << i)))
		{
			target.row(rowCounter) = m_target.row(i);
			++rowCounter;
		}
	}

	double norm = arma::norm((target - value), 2);

	if (0 < m_precision)
	{
		if (norm <= m_precision)
		{
			/* this means target reached */
			value = target;
		}
	}

	return target - value;
}

