/*
 * inverseKinematicsSRJacobian.cpp
 *
 *  Created on: 25.06.2014
 *      Author: lutz
 */

#include "inverseKinematicsSRJacobian.h"

#define DEFAULT_DEG_SUPPRESSION_MIN_LEN 0.001

InverseKinematicsSRJacobian::InverseKinematicsSRJacobian()
	: InverseKinematicJacobian()
	, m_dogSuppressionMinLen(DEFAULT_DEG_SUPPRESSION_MIN_LEN)
{
}

InverseKinematicsSRJacobian::~InverseKinematicsSRJacobian() {
}


double InverseKinematicsSRJacobian::iterationSteps(KinematicTree const& tree, std::map<MotorID, Degree>& o_angles, uint iterationCnt)
{
	const uint32_t numTasks = m_tasks.size();

	const uint32_t numCols = tree.getMotorCt();
	if ((0 < numTasks) &&
		(0 < numCols)) /* at least one task and at least one motor */
	{
		std::map<MotorID, Degree> curMotorValues, noisyMotorValues;
		tree.getMotorValues(curMotorValues);

		arma::colvec motorValuesVec = arma::zeros(curMotorValues.size());
		uint i = 0;
		for (std::pair<MotorID, Degree> const& curValue : curMotorValues)
		{
			motorValuesVec(i) = curValue.second.value();
			++i;
		}

		/* calculate the size of the jacobian and the task vector */
		uint32_t numRows = 0;
		for (const KinematicEngineTask* const &task : m_tasks)
		{
			numRows += task->getDimensionCnt();
		}

		/* build the "big" jacobian */
		arma::mat jacobian = arma::zeros(numRows, numCols);

		/* and the "big" error vector */
		arma::colvec errorVec = arma::zeros(numRows);

		for (uint iteration = 0; iteration < iterationCnt; ++iteration)
		{
			uint32_t beginRow = 0;
			for (const KinematicEngineTask *const &task : m_tasks)
			{
				if (task->hasTarget())
				{
					arma::mat U, V;
					arma::mat Sigma;
					arma::colvec s;

					arma::mat jacobianForTask = task->getJacobianForTask(tree);

					arma::svd(U, s, V, jacobianForTask);

					uint maxDim = s.n_rows;
					for (uint i = 0; i < s.n_rows; ++i) {
						if (s(i) < m_dogSuppressionMinLen) {
							maxDim = i;
							break;
						}
					}

					if (maxDim > 0) {
						const uint32_t endRow = beginRow + maxDim - 1;
						const arma::colvec error  = task->getError(tree);

						arma::colvec s_ = s.rows(0, maxDim - 1);
						Sigma = arma::diagmat(s_);
						Sigma.resize(maxDim, numCols);

						jacobianForTask = Sigma * V.t();
						const arma::colvec error_ = U.t() * error;
						const arma::colvec error_hat = error_.rows(0, maxDim - 1);

						jacobian.submat(beginRow, 0, endRow, numCols - 1) = jacobianForTask;
						errorVec.rows(beginRow, endRow) = error_hat * task->getWeight();

						beginRow = endRow + 1;
					}
				}
			}

//			printf("new size of jacobian: %d, %d\n", beginRow, numCols);

			jacobian.resize(beginRow, numCols);
			errorVec.resize(beginRow);

			/* build the pseudo inverse jacobian */
			arma::mat helper = jacobian * jacobian.t();

			double epsilon = m_epsilon;

			const arma::mat I = arma::eye(helper.n_rows, helper.n_cols);

			if (0 == arma::det(helper + m_epsilon * I))
			{
				/*increase epsilon to be more robust against singularities */
				epsilon *= 2;
			}

//			std::cout.precision(3);
//			std::cout << std::fixed << std::setw(7);

//			jacobian.raw_print(std::cout); printf("\n\n\n");
			errorVec.print(); printf("\n");

			const arma::mat pseudoInverseJacobian = jacobian.t() * arma::inv((helper + epsilon * I));
			arma::colvec jointAngleDiffs = pseudoInverseJacobian * errorVec;

			for (uint32_t i = 0; i < jointAngleDiffs.n_rows; ++i)
			{
				jointAngleDiffs(i) = Math::limited(jointAngleDiffs(i), -m_maxValueChange, m_maxValueChange);
			}


			const arma::colvec newJointAngles = motorValuesVec + jointAngleDiffs;

			for (i = 0; i < jointAngleDiffs.n_rows; ++i)
			{
				if (jointAngleDiffs(i) != 0.)
				{
					Radian newAngle = Radian(motorValuesVec(i) * degrees) + jointAngleDiffs(i) * radians;

					/* clip */
					MotorID id = tree.toExt(i);
					Degree newAngleDeg = tree.clipAngleForMotor(id, Degree(newAngle));
//					tree.setMotorValue(id, newAngleDeg);
					o_angles[id] = newAngleDeg;
				}
			}
		}

		return arma::dot(errorVec, errorVec);
	} else
	{
//			WARNING("InverseKinematicJacobian has no tasks %d or task are malformed %d", numTasks, numCols);
	}

	return 0;
}
