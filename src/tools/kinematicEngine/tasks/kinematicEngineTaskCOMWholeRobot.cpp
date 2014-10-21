/*
 * kinematicEngineTaskCOMWholeRobot.cpp
 *
 *  Created on: 28.02.2014
 *      Author: lutz
 */

#include <tools/kinematicEngine/tasks/kinematicEngineTaskCOMWholeRobot.h>

KinematicEngineTaskCOMWholeRobot::KinematicEngineTaskCOMWholeRobot() : KinematicEngineTask(),
	m_subDimension(DIMENSION_X | DIMENSION_Y | DIMENSION_Z),
	m_dimCnt(3)
{
}

KinematicEngineTaskCOMWholeRobot::KinematicEngineTaskCOMWholeRobot(std::string name, MotorID base, KinematicTree const &tree,  SubDimension subDim) :
		KinematicEngineTask(name, base, EFFECTOR_ID_ROOT, tree),
		m_subDimension(subDim)
{
	m_dimCnt = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (0 != (m_subDimension & (1 << i)))
		{
			++m_dimCnt;
		}
	}

	if (0 == m_dimCnt) {
		ERROR("Cannot create task with no specified dimensions!");
	}
}

KinematicEngineTaskCOMWholeRobot::~KinematicEngineTaskCOMWholeRobot()
{
}

/**
 * get the jacobian for this task
 * @param kinematicTree
 * @return
 */
arma::mat KinematicEngineTaskCOMWholeRobot::getJacobianForTask(const KinematicTree &kinematicTree, bool normalizeJacobian) const
{
	arma::mat retJacobian = arma::zeros(m_dimCnt, kinematicTree.getMotorCt());

	if (false != m_hasTarget)
	{
		arma::mat jacobian = arma::zeros(m_target.n_rows, kinematicTree.getMotorCt());

		const KinematicNode *node = kinematicTree.getNode(m_baseNode);
		KinematicMass childrenMasses;
		KinematicMass parentsMass;

		/* get equivalent masses of relatives */
		const KinematicNode *parent = node->getParent();
		if (nullptr != parent)
		{
			arma::mat jacobianFromParent = getJacobianForTaskSub(kinematicTree, parent, node, parentsMass, true);

			arma::mat44 transform = node->getBackwardMatrix();

			jacobianFromParent = transform.submat(0, 0, 2, 2) * jacobianFromParent;
			parentsMass.applyTransformation(transform);

			jacobian += jacobianFromParent;
		}

		for (const KinematicNode* const &child : node->getChildren())
		{
			KinematicMass subMass;
			arma::mat intermediateJacobian = getJacobianForTaskSub(kinematicTree, child, node, subMass, false);

			arma::mat44 transform = child->getForwardMatrix();
			intermediateJacobian = transform.submat(0, 0, 2, 2) * intermediateJacobian;

			subMass.applyTransformation(transform);

			childrenMasses += subMass;
			jacobian += intermediateJacobian;
		}

		/* apply this joint to the jacobian */
		if (false == node->isFixedNode())
		{
			arma::colvec3 partialDerivativeChildren = node->getPartialDerivativeOfLocationToEffector(childrenMasses.m_position) * childrenMasses.m_massGrams / 1000;
			arma::colvec3 partialDerivativeParent   = -node->getPartialDerivativeOfLocationToEffector(parentsMass.m_position) * parentsMass.m_massGrams / 1000;
			arma::colvec3 partialDerivative = partialDerivativeChildren + partialDerivativeParent;
			jacobian.col(kinematicTree.toInt(node->getID())) = partialDerivative;
		}

		removeDOFfromJacobian(jacobian, kinematicTree);
		retJacobian = removeSubdimensions(jacobian, m_subDimension, m_dimCnt);
		if (normalizeJacobian) {
			normJacobian(retJacobian);
		}
	}

	return retJacobian;
}


arma::mat KinematicEngineTaskCOMWholeRobot::getJacobianForTaskSub(const KinematicTree& kinematicTree, const KinematicNode *node, const KinematicNode *prevNode, KinematicMass &o_mass, bool traversingUp) const
{
	arma::mat jacobian = arma::zeros(m_target.n_rows, kinematicTree.getMotorCt());
	KinematicMass childrenMasses;

	/* get equivalent masses of relatives */
	const KinematicNode *parent = node->getParent();

	if (traversingUp) // if so we can only manipulate our parent's masses but need to propagate our childrens masses back to the child we came from
	{
		KinematicMass parentsMass;
		if ((nullptr != parent) &&
			(parent != prevNode)) // if can traverse UP
		{
			KinematicMass subMass; // this is the mass from everything attached to the parent

			arma::mat intermediateJacobian = getJacobianForTaskSub(kinematicTree, parent, node, subMass, true);

			arma::mat44 transform = node->getBackwardMatrix();
			intermediateJacobian = transform.submat(0, 0, 2, 2) * intermediateJacobian;

			subMass.applyTransformation(transform);

			parentsMass += subMass;

			jacobian += intermediateJacobian;
		}

		if (false == node->isFixedNode())
		{
			// negative influence since we are traversing up; meaning that the rotation of effectors is inverse
			arma::colvec3 partialDerivative   = -node->getPartialDerivativeOfLocationToEffector(parentsMass.m_position) * parentsMass.m_massGrams / 1000.;
			jacobian.col(kinematicTree.toInt(node->getID())) = partialDerivative;
		}

		for (const KinematicNode* const &child : node->getChildren())
		{
			if (prevNode != child) // traverse down into every node not visited yet
			{
				KinematicMass subMass;
				arma::mat intermediateJacobian = getJacobianForTaskSub(kinematicTree, child, node, subMass, false);

				arma::mat44 transform = child->getForwardMatrix();
				intermediateJacobian = transform.submat(0, 0, 2, 2) * intermediateJacobian;

				subMass.applyTransformation(transform);
				childrenMasses += subMass;
				jacobian += intermediateJacobian;
			}
		}

		o_mass += parentsMass;
		o_mass += childrenMasses;
		o_mass += node->getEquivalentMass(); // this is how we propagate the node's weight back to the child
	} else
	{
		/* just look into the children*/

		for (const KinematicNode* const &child : node->getChildren())
		{
			if (prevNode != child) // traverse down into every node not visited yet
			{
				KinematicMass subMass;
				arma::mat intermediateJacobian = getJacobianForTaskSub(kinematicTree, child, node, subMass, false);

				arma::mat44 transform = child->getForwardMatrix();
				intermediateJacobian = transform.submat(0, 0, 2, 2) * intermediateJacobian;

				subMass.applyTransformation(transform);
				childrenMasses += subMass;
				jacobian += intermediateJacobian;
			}
		}

		KinematicMass eqMass = node->getEquivalentMass(); // here we can manipulate the mass attached to this joint
		childrenMasses += eqMass;

		if (false == node->isFixedNode())
		{
			arma::colvec3 partialDerivative = node->getPartialDerivativeOfLocationToEffector(childrenMasses.m_position) * childrenMasses.m_massGrams / 1000.;
			jacobian.col(kinematicTree.toInt(node->getID())) = partialDerivative;
		}

		o_mass += childrenMasses;
	}
	return jacobian;
}


arma::colvec KinematicEngineTaskCOMWholeRobot::getError(const KinematicTree &kinematicTree) const
{
	arma::colvec3 actualCOM = kinematicTree.getCOM(m_baseNode).m_position;

	/* get target */
	arma::colvec target = arma::zeros(m_dimCnt);
	arma::colvec value = arma::zeros(m_dimCnt);
	uint8_t rowCounter = 0;
	for (uint i = 0; i < m_target.n_rows; ++i)
	{
		if (0 != (m_subDimension & (1 << i)))
		{
			value.row(rowCounter) = actualCOM.row(i);
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

arma::colvec KinematicEngineTaskCOMWholeRobot::getTargetWithRespectToSubdims(const KinematicTree &kinematicTree) const
{
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
