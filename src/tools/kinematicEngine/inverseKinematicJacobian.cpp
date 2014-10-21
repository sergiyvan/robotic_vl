/*
 * inverseKinematicJacobian.cpp
 *
 *  Created on: 13.06.2014
 *      Author: lutz
 */

#include "inverseKinematicJacobian.h"

#define DEFAULT_EPSILON 1.


InverseKinematicJacobian::InverseKinematicJacobian()
	: InverseKinematics()
	, m_epsilon(DEFAULT_EPSILON)
{
}

InverseKinematicJacobian::~InverseKinematicJacobian()
{
}


double InverseKinematicJacobian::iterationStep(
		KinematicTree const& tree
		, std::map<MotorID, Degree>& o_angles
		, KinematicEngineTasksContainer const& alltasks
		, const KinematicEngineTaskDefaultPosition* idleTask
	) const
{
	double totalError = 0;

	std::map<MotorID, Degree> curMotorValues;
	tree.getMotorValues(curMotorValues);
	uint motorCnt = tree.getMotorCt();

	arma::colvec motorValuesVec = arma::zeros(motorCnt);
	for (std::pair<MotorID, Degree> const& curValue : curMotorValues)
	{
		motorValuesVec(tree.toInt(curValue.first)) = Radian(curValue.second).value();
	}

	arma::colvec summedJointAngleDiffs = arma::zeros(motorCnt);
	arma::mat nullspaceEyeMat = arma::eye(motorCnt, motorCnt);
	arma::mat nullspaceMat = arma::zeros(motorCnt, motorCnt);

	for (uint l(0); l < KinematicEngineTasksTypes::level::NUM_TASK_LEVELS; ++l) {
		std::vector<const KinematicEngineTask*> const& tasks = alltasks[l];

		if (tasks.size() > 0) {
			arma::mat dogMat = nullspaceEyeMat - nullspaceMat;
			arma::mat jacobian = getJacobianForTasks(tree, tasks, true);
			arma::colvec errorVec = getErrorForTasks(tree, tasks);


			/* build the pseudo inverse jacobian */
			arma::mat pseudoInverseJacobian = buildPseudoInverse(jacobian, m_epsilon);

			const arma::colvec jointDiffs = pseudoInverseJacobian * errorVec;
			const arma::colvec jointDiffsDecorr = dogMat * jointDiffs;
			summedJointAngleDiffs += jointDiffsDecorr;

//			std::cout.precision(3);
//			std::cout << std::fixed << std::setw(13);
//			nullspaceMat.raw_print(std::cout); printf("\n");
//			jacobian.raw_print(std::cout); printf("\n");

//			jointDiffs.t().raw_print(std::cout); printf("\n");
//			jointDiffsDecorr.t().raw_print(std::cout); printf("\n");
//			summedJointAngleDiffs.t().raw_print(std::cout); printf("\n");

			nullspaceMat += dogMat * (buildPseudoInverse(jacobian, m_nullspaceEpsilon) * jacobian);

			totalError += arma::dot(errorVec, errorVec);
		}
	}

//	std::cout.precision(3);
//	std::cout << std::fixed << std::setw(7);
//	nullspaceMat.raw_print(std::cout); printf("\n");


	if (nullptr != idleTask) {
		arma::mat helper = arma::mat(nullspaceEyeMat - nullspaceMat);
		arma::colvec errorVec = idleTask->getErrors(tree);
		arma::colvec angleDiffVec = ((nullspaceEyeMat - nullspaceMat) * errorVec);

//		std::cout.precision(3);
//		std::cout << std::fixed << std::setw(7);
//		errorVec.t().raw_print(std::cout);
//		angleDiffVec.t().raw_print(std::cout); printf("\n");

		summedJointAngleDiffs += angleDiffVec;
	}

	for (uint32_t i = 0; i < summedJointAngleDiffs.n_rows; ++i)
	{
		summedJointAngleDiffs(i) = Math::limited(summedJointAngleDiffs(i), -m_maxValueChange, m_maxValueChange);
	}

	motorValuesVec += summedJointAngleDiffs;

	for (uint i = 0; i < motorValuesVec.n_rows; ++i)
	{
		Radian newAngle = motorValuesVec(i) * radians;

		/* clip */
		MotorID id = tree.toExt(i);
		Degree newAngleDeg = tree.clipAngleForMotor(id, Degree(newAngle));
		o_angles[id] = newAngleDeg;
	}

	return totalError;

	return 0;
}

double InverseKinematicJacobian::calculateSpeeds(
		KinematicTree const& tree
		, std::map<MotorID, RPM>& o_speeds
		, KinematicEngineTasksContainer const& tasks
		, const KinematicEngineTaskDefaultPosition* idleTask
	) const
{
	double totalError = 0;
	uint motorCnt = tree.getMotorCt();

	std::map<MotorID, RPM> curMotorSpeeds;
	tree.getMotorSpeeds(curMotorSpeeds);

	arma::colvec motorSpeedsVec = arma::zeros(motorCnt);
	for (std::pair<MotorID, RPM> const& curValue : curMotorSpeeds)
	{
		// convert rounds per minute to radian per second
		motorSpeedsVec(tree.toInt(curValue.first)) = (curValue.second).value() / 60. * (2. * M_PI);
	}

	arma::colvec summedJointSpeedDiffs = arma::zeros(motorCnt);
	arma::mat nullspaceEyeMat = arma::eye(motorCnt, motorCnt);
	arma::mat nullspaceMat = arma::zeros(motorCnt, motorCnt);

	for (uint l(0); l < KinematicEngineTasksTypes::level::NUM_TASK_LEVELS; ++l) {
		std::vector<const KinematicEngineTask*> const& _tasks = tasks[l];

		if (_tasks.size() > 0) {
			arma::mat dogMat = nullspaceEyeMat - nullspaceMat;

			arma::mat jacobian = getJacobianForTasks(tree, _tasks, false);

			// how much "error" (mother function)
			arma::colvec speedTarget = getSpeedTargetForTasks(tree, _tasks);

			// how much "error" derivative (speed)
			arma::colvec speedVec = jacobian * motorSpeedsVec;

			// the actual error
			arma::colvec speedError = speedTarget - speedVec;

//			const arma::colvec jointDiffs = jacobian.t() * speedError;
			const arma::colvec jointDiffs = buildPseudoInverse(jacobian, m_speedEpsilon) * speedError;
//			const arma::colvec jointDiffs = buildPseudoInverse(jacobian, m_speedEpsilon) * speedTarget - motorSpeedsVec;

			summedJointSpeedDiffs += dogMat * jointDiffs;

//			std::cout.precision(6);
//			std::cout << std::fixed << std::setw(15);
//			nullspaceMat.raw_print(std::cout); printf("\n");
//			jacobian.raw_print(std::cout); printf("\n");
//			speedTarget.t().raw_print(std::cout);
//			speedVec.t().raw_print(std::cout);
//			speedError.t().raw_print(std::cout);
//			motorSpeedsVec.t().raw_print(std::cout);
//			summedJointSpeedDiffs.t().raw_print(std::cout);
//			printf("\n");


			nullspaceMat += dogMat * (buildPseudoInverse(jacobian, m_nullspaceEpsilon) * jacobian);

			totalError += arma::dot(speedError, speedError);
		}
	}


//	std::cout.precision(3);
//	std::cout << std::fixed << std::setw(10);
//	nullspaceMat.raw_print(std::cout); printf("\n\n\n");

	if (nullptr != idleTask) {
		arma::mat helper = arma::mat(nullspaceEyeMat - nullspaceMat);
		arma::colvec errorVec = idleTask->getErrors(tree);
		summedJointSpeedDiffs += (helper * errorVec) * idleTask->getSpeed();
	}


//	motorSpeedsVec.t().raw_print(std::cout);
//	summedJointSpeedDiffs.t().raw_print(std::cout);

	summedJointSpeedDiffs += motorSpeedsVec;

	for (uint i = 0; i < summedJointSpeedDiffs.n_rows; ++i)
	{
		RPM newSpeed = summedJointSpeedDiffs(i) * 60. / (2. * M_PI) * rounds_per_minute;

		MotorID id = tree.toExt(i);
		o_speeds[id] = newSpeed;
	}

	return totalError;
}

double InverseKinematicJacobian::iterationStepGravitation(KinematicTree const& tree, std::map<MotorID, double>& o_torques, std::vector<const KinematicEngineTask*> const& tasks) const
{
	const uint32_t numTasks = tasks.size();

	if (numTasks > 0)
	{
		const uint32_t numCols = tree.getMotorCt();

		/* calculate the size of the jacobian and the task vector */
		uint32_t numRows = 0;
		for (const KinematicEngineTask* const &task : tasks)
		{
			if (task->hasTarget()) {
				numRows += task->getDimensionCnt();
			}
		}

		/* build the "big" jacobian */
		arma::mat jacobian = arma::zeros(numRows, numCols);

		/* and the "big" error vector */
		arma::colvec errorVec = arma::zeros(numRows);

		uint32_t beginRow = 0;
		for (const KinematicEngineTask *const &task : tasks)
		{
			if (task->hasTarget())
			{
				const uint32_t endRow = beginRow + task->getDimensionCnt() - 1;
				const arma::colvec error  = task->getTargetWithRespectToSubdims(tree);

				arma::mat jacobianForTask = task->getJacobianForTask(tree, false);

				jacobian.submat(beginRow, 0, endRow, numCols - 1) = jacobianForTask;

				errorVec.rows(beginRow, endRow) = error * task->getWeight();

				beginRow = endRow + 1;
			}
		}

//			std::cout.precision(3);
//			std::cout << std::fixed << std::setw(7);
//			jacobian.raw_print(std::cout); printf("\n\n\n");
//			errorVec.t().print(); printf("\n");

//		arma::mat helper = jacobian * jacobian.t();
//		const arma::mat pseudoInverseJacobian = jacobian.t() * arma::inv((helper + m_epsilon * arma::eye(helper.n_rows, helper.n_cols)));
		const arma::mat pseudoInverseJacobian = jacobian.t();

		arma::colvec torques = pseudoInverseJacobian * errorVec;

		for (uint i = 0; i < torques.n_rows; ++i)
		{
			MotorID id = tree.toExt(i);
			o_torques[id] = torques(i);
		}

		return arma::dot(errorVec, errorVec);
	}

	return 0;
}

arma::mat InverseKinematicJacobian::getJacobianForTasks(KinematicTree const& tree, std::vector<const KinematicEngineTask*> const& tasks, bool normalize) const
{
	const uint32_t numTasks = tasks.size();
	uint numCols = tree.getMotorCt();

	arma::mat jacobian = arma::zeros(1, numCols);
	if ((0 < numTasks) &&
		(0 < numCols)) /* at least one task and at least one motor */
	{
		/* calculate the size of the jacobian and the task vector */
		uint32_t numRows = 0;
		for (const KinematicEngineTask* const &task : tasks)
		{
			if (task->hasTarget()) {
				numRows += task->getDimensionCnt();
			}
		}

		/* build the "big" jacobian */
		jacobian = arma::zeros(numRows, numCols);

		uint32_t beginRow = 0;
		for (const KinematicEngineTask *const &task : tasks)
		{
			if (task->hasTarget())
			{
				const uint32_t endRow = beginRow + task->getDimensionCnt() - 1;

				arma::mat jacobianForTask = task->getJacobianForTask(tree, normalize) * 1. / task->getWeight();
				jacobian.submat(beginRow, 0, endRow, numCols - 1) = jacobianForTask;

				beginRow = endRow + 1;
			}
		}
	}

	return jacobian;
}

arma::colvec InverseKinematicJacobian::getErrorForTasks(KinematicTree const& tree, std::vector<const KinematicEngineTask*> const& tasks) const
{
	const uint32_t numTasks = tasks.size();

	arma::colvec errorVec = arma::zeros(1);
	if (0 < numTasks) /* at least one task */
	{
		uint32_t numRows = 0;
		for (const KinematicEngineTask* const &task : tasks)
		{
			if (task->hasTarget()) {
				numRows += task->getDimensionCnt();
			}
		}
		errorVec = arma::zeros(numRows);

		uint32_t beginRow = 0;
		for (const KinematicEngineTask *const &task : tasks)
		{
			if (task->hasTarget())
			{
				const uint32_t endRow = beginRow + task->getDimensionCnt() - 1;
				arma::colvec error = task->getError(tree);
				errorVec.rows(beginRow, endRow) = error * task->getWeight();
				beginRow = endRow + 1;
			}
		}
	}

	return errorVec;
}

arma::colvec InverseKinematicJacobian::getSpeedTargetForTasks(KinematicTree const& tree, std::vector<const KinematicEngineTask*> const& tasks) const
{
	const uint32_t numTasks = tasks.size();

	arma::colvec errorVec = arma::zeros(1);
	if (0 < numTasks) /* at least one task */
	{
		uint32_t numRows = 0;
		for (const KinematicEngineTask* const &task : tasks)
		{
			if (task->hasTarget()) {
				numRows += task->getDimensionCnt();
			}
		}
		errorVec = arma::zeros(numRows);

		uint32_t beginRow = 0;
		for (const KinematicEngineTask *const &task : tasks)
		{
			if (task->hasTarget())
			{
				const uint32_t endRow = beginRow + task->getDimensionCnt() - 1;
				arma::colvec error = task->getError(tree) * task->getWeight();

				if (task->hasSpeed() && arma::norm(error, 2) > 0.000000001 ) {
					error *= task->getSpeedToReachTarget() / arma::norm(error, 2);
				}

				errorVec.rows(beginRow, endRow) = error;
				beginRow = endRow + 1;
			}
		}
	}

	return errorVec;
}

int InverseKinematicJacobian::calculateNumRows(std::vector<const KinematicEngineTask*> const& tasks) const
{
	int numRowsConstraints = 0;
	for (const KinematicEngineTask* const &task : tasks)
	{
		if (task->hasTarget()) {
			numRowsConstraints += task->getDimensionCnt();
		}
	}
	return numRowsConstraints;
}


arma::mat InverseKinematicJacobian::buildPseudoInverse(arma::mat matrix, double epsilon) const
{
	arma::mat helper = matrix * matrix.t();
	const arma::mat I = arma::eye(helper.n_rows, helper.n_cols);
	arma::mat pseudoInverseJacobian = matrix.t() * arma::inv((helper + epsilon * I));
	return pseudoInverseJacobian;
}

