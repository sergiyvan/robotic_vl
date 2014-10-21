/*
 * kinematicEngineTaskLinearMomentum.cpp
 *
 *  Created on: 08.08.2014
 *      Author: lutz
 */

#include <tools/kinematicEngine/tasks/kinematicEngineTaskLinearMomentum.h>

KinematicEngineTaskLinearMomentum::KinematicEngineTaskLinearMomentum()  : KinematicEngineDynamicTask(),
	m_subDimension(DIMENSIONS_ALL),
	m_dimCnt(3) {
}

KinematicEngineTaskLinearMomentum::~KinematicEngineTaskLinearMomentum() {
}

KinematicEngineTaskLinearMomentum::KinematicEngineTaskLinearMomentum(std::string name, MotorID base, KinematicTree const &tree, SubDimension subDim)
	: KinematicEngineDynamicTask(name, base, EFFECTOR_ID_ROOT, tree)
	, m_subDimension(subDim) {
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

arma::mat KinematicEngineTaskLinearMomentum::getJacobianForTask(const KinematicTree &kinematicTree, bool normalizeJacobian) const {
	arma::mat retJacobian = arma::zeros(m_dimCnt, kinematicTree.getMotorCt());

	if (false != m_hasTarget)
	{
		arma::mat jacobian = arma::zeros(m_target.n_rows, kinematicTree.getMotorCt());

		const KinematicNode *node = kinematicTree.getNode(m_baseNode);

		KinematicMass massFromParent, massFromChildren;

		for (const KinematicNode* child: node->getChildren()) {
			jacobian += child->getForwardMatrix().submat(0, 0, 2, 2) * getJacobianForTaskSub(kinematicTree, node, false, massFromChildren);
		}

		jacobian += getJacobianForTaskSub(kinematicTree, node, true, massFromChildren);

		removeDOFfromJacobian(jacobian, kinematicTree);
		retJacobian = removeSubdimensions(jacobian, m_subDimension, m_dimCnt);
		if (normalizeJacobian) {
			normJacobian(retJacobian);
		}
	}

	return retJacobian;
}

arma::mat KinematicEngineTaskLinearMomentum::getJacobianForTaskSub(const KinematicTree &kinematicTree,
				const KinematicNode *node,
				bool traversingUp,
				KinematicMass &o_massFromSubTree) const
{
	KinematicMass attachedNodesMass;
	arma::mat retJacobian = arma::zeros(m_target.n_rows, kinematicTree.getMotorCt());
	if (traversingUp) {
		const KinematicNode* parent = node->getParent();
		if (nullptr != parent) {
			const arma::mat33 parentToNodeRotMat = parent->getBackwardMatrix().submat(0, 0, 2, 2);
			retJacobian = parentToNodeRotMat * getJacobianForTaskSub(kinematicTree, node->getParent(), true, attachedNodesMass);

			for (const KinematicNode* sibling : parent->getChildren()) {
				if (sibling != node) {
					retJacobian += parentToNodeRotMat * sibling->getForwardMatrix().submat(0, 0, 2, 2) * getJacobianForTaskSub(kinematicTree, sibling, false, attachedNodesMass);
				}
			}

			attachedNodesMass += parent->getEquivalentMass();
			attachedNodesMass.applyTransformation(node->getBackwardMatrix());
			o_massFromSubTree += attachedNodesMass;

			if (!node->isFixedNode()) {
				const int idx = kinematicTree.toInt(node->getID());
				arma::colvec4 momentumDerivative = node->getPartialDerivativeOfLocationToEffector(attachedNodesMass.m_position) * attachedNodesMass.m_massGrams;
				retJacobian.col(idx) = momentumDerivative.rows(0, 2);
			}
		}
	} else {
		for (const KinematicNode *child : node->getChildren()) {
			retJacobian += child->getForwardMatrix().submat(0, 0, 2, 2) * getJacobianForTaskSub(kinematicTree, child, false, attachedNodesMass);
		}

		if (!node->isFixedNode()) {
			const int idx = kinematicTree.toInt(node->getID());
			KinematicMass curNodesMass = attachedNodesMass + node->getEquivalentMass();
			arma::colvec4 momentumDerivative = node->getPartialDerivativeOfLocationToEffector(curNodesMass.m_position) * curNodesMass.m_massGrams;

			retJacobian.col(idx) = momentumDerivative.rows(0, 2);

			curNodesMass.applyTransformation(node->getForwardMatrix());
			o_massFromSubTree += curNodesMass;
		}
	}
	return retJacobian;
}

arma::colvec KinematicEngineTaskLinearMomentum::getError(const KinematicTree &kinematicTree) const {
	arma::colvec3 actualLinMomentum = kinematicTree.getLinearMomentum(m_baseNode);

	/* get target */
	arma::colvec target = arma::zeros(m_dimCnt);
	arma::colvec value = arma::zeros(m_dimCnt);
	uint8_t rowCounter = 0;
	for (uint i = 0; i < m_target.n_rows; ++i)
	{
		if (0 != (m_subDimension & (1 << i)))
		{
			value.row(rowCounter) = actualLinMomentum.row(i);
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
