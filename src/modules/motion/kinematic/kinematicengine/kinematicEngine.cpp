/*
 * kinematicEngine.cpp
 *
 *  Created on: 09.02.2014
 *      Author: lutz
 */

#include <modules/motion/kinematic/kinematicengine/kinematicEngine.h>
#include "modules/motion/motion.h"
#include "management/config/config.h"
#include <utils/math/Math.h>

namespace {
	auto cfgSection          = ConfigRegistry::getSection("kinematicengine");
	auto cfgEpsilon          = cfgSection->registerOption<double>("epsilon",      0.1, "the epsilon used in the regularization term for the pseudoinverse jacobian");
	auto cfgSpeedEpsilon     = cfgSection->registerOption<double>("speedepsilon",      0.01, "the epsilon used in the regularization term for the pseudoinverse jacobian (used for calculation of speeds");
	auto cfgNullspaceEpsilon = cfgSection->registerOption<double>("nullspaceepsilon",      0.00000001, "the epsilon used in the regularization term for the nullspace matrix calculation");
	auto cfgMaxAngleDiff     = cfgSection->registerOption<double>("maxAngleDiff", 45., "the maximum angle change to a motor that can be done in one iteration (degrees)");
	auto cfgNoiseAngle       = cfgSection->registerOption<double>("angularNoise", 0.01, "angular noise which can be applied if a task is singular (degree)");
	auto cfgIterationCnt     = cfgSection->registerOption<int>   ("iterationCnt", 3, "how often iterate the inverse kinematics per motion cycle");
}

static std::string activeTasksName = "motion.kinematicengine.activeTasks";
REGISTER_DEBUG(activeTasksName, TABLE, BASIC);

REGISTER_MODULE(Motion, KinematicEngine, false, "Kinematic engine for multiple movement tasks and cool stuff. Can save the world")

KinematicEngine::KinematicEngine()
	: m_inverseKinematic()
{
}

KinematicEngine::~KinematicEngine() {
}


void KinematicEngine::init()
{
	services.getEvents().registerForEvent(EVT_CONFIGURATION_LOADED, this);
	eventCallback(EVT_CONFIGURATION_LOADED, &services.getConfig());
}


void KinematicEngine::eventCallback(EventType eventType, void*) {
	if (eventType == EVT_CONFIGURATION_LOADED) {
		m_inverseKinematic.setEpsilon(cfgEpsilon->get());
		m_inverseKinematic.setSpeedEpsilon(cfgSpeedEpsilon->get());
		m_inverseKinematic.setNullspaceEpsilon(cfgNullspaceEpsilon->get());
		m_inverseKinematic.setMaxValueChange(cfgMaxAngleDiff->get());
		distribution = std::uniform_real_distribution<double>(-cfgNoiseAngle->get(), cfgNoiseAngle->get());
	}
}

void KinematicEngine::execute()
{
	KinematicEngineTasksContainer const& tasks = getKinematicEngineTasks().getTasks();
	KinematicTree &tree = const_cast<KinematicTree&>(getKinematicEngineTasks().getTree());

	tree.setGyroscopeAngles(getGyroData().getRotMat());

	for (auto position : getMotorAngles().getPositions()) {
		const double noise = distribution(generator);
		tree.setMotorValue(position.first, position.second + noise * degrees);
	}

	tree.setMotorSpeeds(getMotorAngles().getSpeeds());

	std::map<MotorID, double> torques;
	m_inverseKinematic.iterationStepGravitation(tree, torques, getKinematicEngineTasks().getGravityTasks());

	std::map<MotorID, RPM> speedsToApply;
	m_inverseKinematic.calculateSpeeds(tree, speedsToApply, tasks, getKinematicEngineTasks().getIdleTask());

	std::map<MotorID, Degree> anglesToSet;
	for (int i = 0; i < cfgIterationCnt->get(); ++i) {
		const KinematicEngineTaskDefaultPosition* idleTask = nullptr;
		if (i == cfgIterationCnt->get() - 1) {
			idleTask = getKinematicEngineTasks().getIdleTask();
		}
		m_inverseKinematic.iterationStep(tree, anglesToSet, tasks, idleTask);
		tree.setMotorValues(anglesToSet);
	}




	Degree helper(cfgMaxAngleDiff->get() * degrees);
	for (std::pair<MotorID, Degree> const& motor : anglesToSet)
	{
		Degree curPosition = getMotorAngles().getPosition(motor.first);
		Degree angleChange = Math::limited(motor.second - curPosition, -helper, helper);

		std::map<MotorID, RPM>::iterator element = speedsToApply.find(motor.first);
		RPM speed = 0 * rounds_per_minute; // default speed
		if (element != speedsToApply.end()) {
			speed = abs(element->second);
//			INFO("setting Speed for %d\t%05.5f", element->first, element->second.value());
		}
		getMotorPositionRequest().setSpeed(motor.first, speed);
		getMotorPositionRequest().setPosition(motor.first, curPosition + angleChange);
	}

	for (std::pair<MotorID, double> const& motor : torques)
	{
		getMotorPositionRequest().setForce(motor.first, motor.second);
	}

	int level = 0;
	for (std::vector<KinematicEngineTask const*> const& tasks : getKinematicEngineTasks().getTasks()) {
		std::stringstream ss;
		ss << level << "\t";
		int subCounter = 0;
		for (KinematicEngineTask const* task : tasks) {
			std::stringstream subLevelStream;
			subLevelStream << ss.str();
			subLevelStream << subCounter;
			std::stringstream values;
			values << task->getName();
			values.precision(5);
			values << std::fixed << std::setw(5);
			arma::colvec errorVec = task->getError(getKinematicTree());
			values << " (" <<  arma::norm(errorVec, 2) << ") " << errorVec.t();
			DEBUG_TABLE(activeTasksName, subLevelStream.str(), values.str());
			++subCounter;
		}
		++level;
	}

	/**
	 * remove all tasks - we are done here
	 */
	const_cast<KinematicEngineTasks&>(getKinematicEngineTasks()).clearTasks();
}
