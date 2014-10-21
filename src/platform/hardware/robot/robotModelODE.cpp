/*
 * robotModelODE.cpp
 *
 *  Created on: 24.09.2014
 *      Author: lutz
 */

#include "robotModel.h"

#include "representations/motion/kinematicTree.h"
#include <tools/kinematicEngine/physics/physicsEnvironment.h>
#include "tools/kinematicEngine/physics/physicsVisualization.h"

#include <platform/hardware/actuators/actuatorsODE.h>
#include <platform/hardware/imu/imuODE.h>
#include <platform/hardware/clock/clockODE.h>

#include <utils/units.h>
#include <platform/system/thread.h>
#include <management/config/config.h>

#include <chrono>
#include <thread>

static const Millisecond minDelay(10. * milliseconds);

namespace {
	auto cfgSection        = ConfigRegistry::getSection("physicssimulator");
	auto cfgSpeedFactor    = cfgSection->registerOption<double>("speedfactor",      1.,   "speedup for the simulation");
	auto cfgVisualsEnabled = cfgSection->registerOption<int>("visuals",             1 ,   "wether or not visuals are enabled");
	auto cfgFramerate      = cfgSection->registerOption<double>("framerate",        100.,  "framerate of the simulation");
	auto cfgPrintFramerate = cfgSection->registerOption<double>("printframerate",   0.2, "framerate of how often to print the actual framerate");
}



class RobotModelODE_pimpl : public Thread {
public:
	RobotModelODE_pimpl(PhysicsEnvironment *environment)
		: Thread()
		, m_envitonment(environment)
	{
	}

	~RobotModelODE_pimpl() {
	}

	virtual const char* getName() const override {
		return "ODE Simulator Thread";
	}

	virtual void threadMain() override {
		Millisecond startTime = getCurrentTime();
		Millisecond timeSimulated = 0 * milliseconds;

		Millisecond timeOnLastFrequencyPrint = startTime;
		int tickCnt = 0;

		while (isRunning()) {
			Millisecond now = getCurrentTime();
			Millisecond timeSinceStart = now - startTime;
			Millisecond interval = Millisecond(1. * seconds / cfgFramerate->get());

			if (0. == cfgSpeedFactor->get()) {
				sleep(1);
				continue;
			}

			double iteration = floor((timeSinceStart + interval / 2) / interval);

			Millisecond timeToSimulate = std::min(10. * interval, interval * (iteration + 1) - timeSimulated);
			m_envitonment->simulateStep(timeToSimulate * cfgSpeedFactor->get());
			timeSimulated += timeToSimulate;
			if (m_clock) {
				m_clock->setCurrentTime(timeSimulated);
			}

			now = getCurrentTime();
			Millisecond timeForNextWakeup = startTime + interval * (iteration + 1);
			Millisecond timeTillNextWakeup = timeForNextWakeup - now;

			delay(timeTillNextWakeup);

			++tickCnt;
			Millisecond frequencyPrintInteval = Millisecond(1. * seconds / cfgPrintFramerate->get());

			if (timeOnLastFrequencyPrint + frequencyPrintInteval < now) {
				INFO("simulator framerate: %fHz", Hertz(double(tickCnt) / frequencyPrintInteval).value());
				timeOnLastFrequencyPrint = now;
				tickCnt = 0;
			}
		}
	}

	void setClock(ClockODE *clock) {
		m_clock = clock;
	}

private:
	Millisecond lastTimestamp;
	PhysicsEnvironment *m_envitonment;
	ClockODE *m_clock;
};


class RobotModelODE : public RobotModel {
public:
	RobotModelODE();
	virtual ~RobotModelODE();

	virtual bool init() override;

private:
	KinematicTree m_tree;
	PhysicsEnvironment m_physicsEnvironment;
	RobotModelODE_pimpl m_simulationThread;
};

REGISTER_ROBOTMODEL("ode", RobotModelODE, "RobotModel for ode (physics simulator)");

RobotModelODE::RobotModelODE()
	: RobotModel()
	, m_physicsEnvironment()
	, m_simulationThread(&m_physicsEnvironment)
{
}

RobotModelODE::~RobotModelODE() {
	m_simulationThread.cancel();
}


bool RobotModelODE::init() {
	if (cfgVisualsEnabled->get()) {
		PhysicsVisualization &visualizaion = PhysicsVisualization::getInstance();
		visualizaion.setEnvironmentToDraw(&m_physicsEnvironment);
	}

	m_tree.setup(*getRobotDescription());
	m_physicsEnvironment.setKinematicModel(&m_tree);

	actuators  = std::move(std::unique_ptr<ActuatorsODE>(new ActuatorsODE(&m_physicsEnvironment, &m_tree)));
	imu        = std::move(std::unique_ptr<IMU_ODE>(new IMU_ODE(this, &m_physicsEnvironment, &m_tree)));
	clock      = std::move(std::unique_ptr<ClockODE>(new ClockODE()));

	m_simulationThread.setClock((ClockODE*)getClock());

	hardwareIsInitialized = true;


	bool ret = RobotModel::init();

	// let the simulator run
	m_simulationThread.run();

	return ret;
}
