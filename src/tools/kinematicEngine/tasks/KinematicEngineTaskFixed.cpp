/*
 * KinematicEngineTaskFixed.cpp
 *
 *  Created on: 17.06.2014
 *      Author: lutz
 */

#include "KinematicEngineTaskFixed.h"


KinematicEngineTaskFixed::KinematicEngineTaskFixed()
	: KinematicEngineTask()
	, m_dimCnt(9)
{
	m_target = arma::zeros(m_dimCnt);
}

KinematicEngineTaskFixed::KinematicEngineTaskFixed(std::string name,
													MotorID baseNode,
													MotorID effectorNode,
													const KinematicTree &tree)
	: KinematicEngineTask(name, baseNode, effectorNode, tree)
	, m_locationTask(name, baseNode, effectorNode, tree)
	, m_rotationTaskX(name, baseNode, effectorNode, tree, KinematicEngineTaskOrientation::AXIS_X)
	, m_rotationTaskY(name, baseNode, effectorNode, tree, KinematicEngineTaskOrientation::AXIS_Y)
{
	m_dimCnt = 9;
	m_target = arma::zeros(m_dimCnt);

	const arma::mat44 transition = tree.getTransitionMatrixFromTo(baseNode, effectorNode);

	m_rotationTaskX.setTarget(transition.col(0).rows(0, 2));
	m_rotationTaskY.setTarget(transition.col(1).rows(0, 2));
	m_locationTask.setTarget(transition.col(3).rows(0, 2));

	m_target.rows(0, 2) = m_rotationTaskX.getTarget();
	m_target.rows(3, 5) = m_rotationTaskY.getTarget();
	m_target.rows(6, 8) = m_locationTask.getTarget();

	m_hasTarget = true;
}

KinematicEngineTaskFixed::~KinematicEngineTaskFixed() {
}

arma::mat KinematicEngineTaskFixed::getJacobianForTask(const KinematicTree &tree, bool normJacobian) const
{
	arma::mat retJacobian = arma::zeros(m_dimCnt, tree.getMotorCt());
	if (m_hasTarget)
	{
		retJacobian.rows(0, 2) = m_rotationTaskX.getJacobianForTask(tree, normJacobian);
		retJacobian.rows(3, 5) = m_rotationTaskY.getJacobianForTask(tree, normJacobian);
		retJacobian.rows(6, 8) = m_locationTask.getJacobianForTask(tree, normJacobian);
	}

	return retJacobian;
}

arma::colvec KinematicEngineTaskFixed::getError(const KinematicTree &tree) const
{
	arma::mat error = arma::zeros(m_dimCnt);
	if (m_hasTarget)
	{
		error.rows(0, 2) = m_rotationTaskX.getError(tree);
		error.rows(3, 5) = m_rotationTaskY.getError(tree);
		error.rows(6, 8) = m_locationTask.getError(tree);
	}
	return error;
}
