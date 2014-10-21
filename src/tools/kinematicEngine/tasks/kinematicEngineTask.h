/*
 * kinematicEngineTask.h
 *
 *  Created on: 09.02.2014
 *      Author: lutz
 */

#ifndef KINEMATICENGINETASK_H_
#define KINEMATICENGINETASK_H_

#include "representations/motion/kinematicTree.h"
#include <armadillo>
#include <vector>
#include <utility>

class KinematicEngineTask {

protected:

	/* default constructor */
	KinematicEngineTask() :
		m_name("uninitialized"),
		m_baseNode(EFFECTOR_ID_ROOT),
		m_effectorNode(EFFECTOR_ID_ROOT),
		m_target(arma::zeros(3)),
		m_speed(0.),
		m_actuatorsToIgnore(),
		m_hasTarget(false),
		m_hasSpeed(false),
		m_weight(0),
		m_precision(0.),
		m_invPath(),
		m_path()
	{}

	KinematicEngineTask(std::string name, MotorID baseNode, MotorID effectorNode, const KinematicTree &tree) :
		m_name(name),
		m_baseNode(baseNode),
		m_effectorNode(effectorNode),
		m_target(arma::zeros(3)),
		m_speed(0.),
		m_actuatorsToIgnore(),
		m_hasTarget(false),
		m_hasSpeed(false),
		m_weight(1.),
		m_precision(0.),
		m_invPath(tree.getPathFromNodeToNode(baseNode, effectorNode)),
		m_path(m_invPath)
	{
		m_invPath = invertKinematicPath(m_path);
	}

	KinematicEngineTask(std::string name, MotorID baseNode, MotorID effectorNode, const KinematicTree &tree, arma::colvec3 target) :
		m_name(name),
		m_baseNode(baseNode),
		m_effectorNode(effectorNode),
		m_target(target),
		m_speed(0.),
		m_actuatorsToIgnore(),
		m_hasTarget(true),
		m_hasSpeed(false),
		m_weight(1.),
		m_precision(0.),
		m_invPath(tree.getPathFromNodeToNode(baseNode, effectorNode)),
		m_path(m_invPath)
	{
		m_invPath = invertKinematicPath(m_path);
	}

public:

	virtual ~KinematicEngineTask();

	std::string getName() const {
		return m_name;
	}

	virtual void setTarget(arma::colvec3 target) {
		m_target = target;
		m_hasTarget = true;
	}

	virtual void setSpeedToReachTarget(double targetSpeed) {
		m_speed = targetSpeed;
		m_hasSpeed = true;
	}

	virtual double getSpeedToReachTarget() const {
		return m_speed;
	}

	void setActuatorsToIgnore(const std::vector<MotorID> &actuatorsToIgnore) {
		m_actuatorsToIgnore = actuatorsToIgnore;
	}

	void addActuatorToIgnore(const MotorID &actuatorToIgnore) {
		m_actuatorsToIgnore.push_back(actuatorToIgnore);
	}

	void setWeight(double weight) {
		m_weight = weight;
	}

	void setPrecision(double precision) {
		m_precision = precision;
	}

	virtual arma::colvec getError(const KinematicTree &kinematicTree) const = 0;
	virtual arma::colvec getTargetWithRespectToSubdims(const KinematicTree &kinematicTree) const = 0;

	virtual uint32_t getDimensionCnt() const {
		uint32_t ret = 0;
		if (false != m_hasTarget)
		{
			ret = m_target.n_rows;
		}
		return ret;
	}

	const std::vector<MotorID> &getActuatorsToIgnore() const {
		return m_actuatorsToIgnore;
	}

	bool hasTarget() const {
		return m_hasTarget;
	}

	bool hasSpeed() const {
		return m_hasSpeed;
	}

	double getWeight() const {
		return m_weight;
	}

	/**
	 * get the jacobian for this task
	 * @param kinematicTree
	 * @return
	 */
	virtual arma::mat getJacobianForTask(const KinematicTree &kinematicTree, bool normalizeJacobian = false) const = 0;

	KinematicPath &getPath()
	{
		return m_invPath;
	}

	const KinematicPath &getPath() const
	{
		return m_invPath;
	}

	void setPath(KinematicPath path)
	{
		m_invPath = path;
	}

	arma::colvec getTarget() const
	{
		return m_target;
	}


protected:

	/**
	 * perform the normalization and removal of degrees of freedom
	 * @param jacobian
	 */
	inline void removeDOFfromJacobian(arma::mat &jacobian, const KinematicTree& kinematicTree) const {
		/* remove all motors from the jacobian which shall not be used */
		for (MotorID const &id : m_actuatorsToIgnore)
		{
			jacobian.col(kinematicTree.toInt(id)).zeros();
		}
	}

	inline arma::mat removeSubdimensions(arma::mat &jacobian, int activeSubdimensions, uint dimCnt) const {
		arma::mat retJacobian = arma::zeros(dimCnt, jacobian.n_cols);

		uint o_rowSel = 0;
		uint i_rowSel = 0;
		while (o_rowSel < dimCnt && i_rowSel < jacobian.n_rows)
		{
			if (0 != (activeSubdimensions & (1 << i_rowSel)))
			{
				retJacobian.row(o_rowSel) = jacobian.row(i_rowSel);
				++o_rowSel;
			}
			++i_rowSel;
		}

		return retJacobian;
	}

	inline void normJacobian(arma::mat &jacobian) const
	{
		double maxNorm = 0.;
		for (uint i = 0; i < jacobian.n_cols; ++i)
		{
			maxNorm = std::max(maxNorm, arma::dot(jacobian.col(i), jacobian.col(i)));
		}
		if (maxNorm > 0.)
		{
			maxNorm = sqrt(maxNorm);
			jacobian = 1. / maxNorm * jacobian;
		}
	}

	std::string m_name;

	/**
	 * Id of the node in the kinematic tree in which coordinate frame the task is to be accomplished
	 * All actuators from this node to the effector node will be part of the solution!
	 * If some actuators in that chain shall not be used specify so in the actuatorsToIgnore list.
	 */
	MotorID m_baseNode;

	/**
	 * id of the node to use as effector
	 * If you want to move several actuators you have to create several tasks
	 */
	MotorID m_effectorNode;

	/**
	 * target position or orientation of this task
	 */
	arma::colvec m_target;

	/**
	 * target speed to reach the target
	 */
	double m_speed;

	/**
	 * list of motors which will NOT be used to accheive this task.
	 * Usefull when specifying a task from a coordinate frame (eg. camera) which should not be moved
	 * example: Move the hand to a location in the cameras coordinate frame but do not move the camera
	 * explanation: a valid solution to the task mentioned is to move the cameras coordinate frame (moving the camera) to acceive the goal more easily
	 */
	std::vector<MotorID> m_actuatorsToIgnore;

	/**
	 * flag indicating that this task can be done (since it has a target)
	 */
	bool m_hasTarget;

	/**
	 * flag indicating that this task knows the speed which should be used to reach the target
	 */
	bool m_hasSpeed;

	/**
	 * the weight (priority to accomplish this task)
	 * value between 0 and 1
	 * 0 means do not accomplish this task at all and 1 means the opposite
	 */
	double m_weight;

	/**
	 * when the distance between the target and the current value of the task (eg. proximity) is below precision this task will not be taken into account for motionPlaning
	 * positive value expressed in mm for position tasks;
	 * positive value expressed in scalar product of orientation vector by actual orientation for orientation tasks
	 */
	double m_precision;


	/**
	 * the path through the kinematic tree (stored here to not recompute it every iteration)
	 * order: endeffector to base
	 *        endeffector is the first element
	 */
	KinematicPath m_invPath;

	/**
	 * the path through the kinematic tree (stored here to not recompute it every iteration)
	 * order: base to endeffector
	 *        base is the first element
	 */
	KinematicPath m_path;
};

typedef KinematicEngineTask KinematicEngineDynamicTask;

#endif /* KINEMATICENGINETASK_H_ */
