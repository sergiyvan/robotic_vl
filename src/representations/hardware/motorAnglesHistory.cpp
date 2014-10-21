#include "motorAnglesHistory.h"
#include "services.h"
#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"

const static size_t MAXMOTORANGLESHISTORY(100);

void MotorAnglesHistory::addMotorAngles(MotorAngles const& data) {
	motorAnglesList.push_front(data);
	while (motorAnglesList.size() > MAXMOTORANGLESHISTORY) {
		motorAnglesList.pop_back();
	}
}


/*------------------------------------------------------------------------------------------------*/

MotorAngles MotorAnglesHistory::getMotorAngles(robottime_t timestamp) const {
	// 1. Case, no Values are available
	if (motorAnglesList.empty()) {
		std::map<MotorID, Degree>  positions;
		std::map<MotorID, Degree>  offsets;
		std::map<MotorID, RPM>     speeds;
		for (auto id : services.getRobotModel().getRobotDescription()->getMotorIDs()) {
			positions[id] = 0*degrees;
			offsets[id]   = 0*degrees;
			speeds[id]    = 0*rounds_per_minute;
		}
		MotorAngles m;
		m.setValues(0, positions, offsets, speeds);
		return m;

	}

	// 2. Case, the newest Values arn't new enough
	if (motorAnglesList.front().getTimestamp() < timestamp)
		return motorAnglesList.front();

	auto iter = motorAnglesList.begin();
	for (; iter != motorAnglesList.end(); ++iter) {
		if (iter->getTimestamp() < timestamp) break;
	}

	// No value found
	if (iter == motorAnglesList.end()) return motorAnglesList.back();
	auto nextIter = iter++;
	if (iter == motorAnglesList.end()) return *nextIter;

	return MotorAngles::getDiff(*iter, *nextIter, timestamp);
}

