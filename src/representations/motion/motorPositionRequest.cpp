#include "motorPositionRequest.h"

#include "debug.h"

#include "services.h"
#include "platform/hardware/robot/robotModel.h"
#include "platform/hardware/robot/robotDescription.h"


void MotorPositionRequest::clear(bool _totalClear) {
	for (auto& e : positions) {
		e.second.second = _totalClear?0:std::max(0, e.second.second-1);
	}
	for (auto& e : offsets) {
		e.second.second = false;
	}
	for (auto& e : speeds) {
		e.second.second = false;
	}
	for (auto& e : forces) {
		e.second.second = false;
	}
	for (auto& e : torques) {
		e.second.second = false;
	}
}

void MotorPositionRequest::setPosition(MotorID _id, Degree _angle) {
	if (positions.find(_id) != positions.end() && positions.at(_id).second == 5) {
//		WARNING("Overwriting position of motor %d (%s) with value %.1f degrees (old value: %f.1f)",
//				_id,
//				services.getRobotModel().getRobotDescription()->getMotorName(_id).c_str(),
//				getMotorName(_id).c_str(),
//				_angle.value(),
//				positions.at(_id).first.value());
	}
	positions[_id] = {_angle, 5};
}
void MotorPositionRequest::setOffset(MotorID _id, Degree _offsetAngle) {
	if (offsets.find(_id) != offsets.end() && offsets.at(_id).second) {
		WARNING("Overwriting offset of motor %d (%s) with value %f degrees (old: value: %f)",
				_id,
				services.getRobotModel().getRobotDescription()->getMotorName(_id).c_str(),
				_offsetAngle.value(),
				offsets.at(_id).first.value());
	}
	offsets[_id] = {_offsetAngle, true};
}
void MotorPositionRequest::setSpeed(MotorID _id, RPM _speed) {
	if (speeds.find(_id) != speeds.end() && speeds.at(_id).second) {
		WARNING("Overwriting speed of motor %d (%s) with value %f rounds per minute",
				_id,
				services.getRobotModel().getRobotDescription()->getMotorName(_id).c_str(),
				_speed.value());
	}
	speeds[_id] = {_speed, true};
}


void MotorPositionRequest::setTorque(MotorID _id, bool _on) {
	auto iter = torques.find(_id);
	if (iter != torques.end() && iter->second.second) {
		WARNING("Overwriting torque of motor %d (%s) with value %d",
				_id,
				services.getRobotModel().getRobotDescription()->getMotorName(_id).c_str(),
				_on);
	}
	if (iter == torques.end() || iter->second.first != _on) {
		torques[_id] = {_on, true};
	}
}
void MotorPositionRequest::setPositionAndSpeed(MotorID _id, Degree _angle, RPM _speed) {
	setPosition(_id, _angle);
	setSpeed(_id, _speed);
}

void MotorPositionRequest::setForce(MotorID _id, double force)
{
	if (forces.find(_id) != forces.end() && forces.at(_id).second) {
		WARNING("Overwriting force of motor %d (%s) with value %f",
				_id,
				services.getRobotModel().getRobotDescription()->getMotorName(_id).c_str(),
				force);
	}
	forces[_id] = {force, true};
}

void MotorPositionRequest::setAllTorques(bool _on) {
	for (auto id : services.getRobotModel().getRobotDescription()->getMotorIDs()) {
		setTorque(id, _on);
	}
}

Degree MotorPositionRequest::getPosition(MotorID _id) const {
	if (positions.find(_id) == positions.end()) {
		return 0*degrees;
	}
	return positions.at(_id).first;
}
RPM MotorPositionRequest::getSpeed(MotorID _id) const {
	if (speeds.find(_id) == speeds.end()) {
		return 0*rounds_per_minute;
	}

	return speeds.at(_id).first;
}
Degree MotorPositionRequest::getOffset(MotorID _id) const {
	if (offsets.find(_id) == offsets.end()) {
		return 0*degrees;
	}
	return offsets.at(_id).first;
}

std::map<MotorID, Degree> MotorPositionRequest::getPositionRequests() const {
	std::map<MotorID, Degree> retMap;
	for (auto e : positions) {
		if (e.second.second > 0) {
			retMap[e.first] = e.second.first;
		}
	}
	return retMap;
}
std::map<MotorID, Degree> MotorPositionRequest::getOffsetRequests() const {
	std::map<MotorID, Degree> retMap;
	for (auto e : offsets) {
		if (e.second.second) {
			retMap[e.first] = e.second.first;
		}
	}
	return retMap;
}
std::map<MotorID, RPM> MotorPositionRequest::getSpeedRequests() const {
	std::map<MotorID, RPM> retMap;
	for (auto e : speeds) {
		if (e.second.second) {
			retMap[e.first] = e.second.first;
		}
	}
	return retMap;

}

std::map<MotorID, double> MotorPositionRequest::getForceRequests() const {
	std::map<MotorID, double> retMap;
	for (auto e : forces) {
		if (e.second.second) {
			retMap[e.first] = e.second.first;
		}
	}
	return retMap;

}

std::map<MotorID, bool> MotorPositionRequest::getTorqueRequests() const {
	std::map<MotorID, bool> retMap;
	for (auto e : torques) {
		if (e.second.second) {
			retMap[e.first] = e.second.first;
		}
	}
	return retMap;
}


void MotorPositionRequest::merge(const MotorPositionRequest& _request) {
	for (auto e : _request.positions) {
		if (e.second.second > 0) {
			setPosition(e.first, e.second.first);
		}
	}
	for (auto e : _request.offsets) {
		if (e.second.second) {
			setOffset(e.first, e.second.first);
		}
	}
	for (auto e : _request.speeds) {
		if (e.second.second) {
			setSpeed(e.first, e.second.first);
		}
	}
	for (auto e : _request.torques) {
		if (e.second.second) {
			setTorque(e.first, e.second.first);
		}
	}
}
void MotorPositionRequest::mergeHeadOnly(const MotorPositionRequest& _request) {
	std::set<MotorID> headIDs = { MOTOR_HEAD_PITCH, MOTOR_HEAD_TURN};

	for (auto e : _request.positions) {
		if (e.second.second > 0 && (headIDs.find(e.first) != headIDs.end())) {
			setPosition(e.first, e.second.first);
		}
	}
	for (auto e : _request.offsets) {
		if (e.second.second && (headIDs.find(e.first) != headIDs.end())) {
			setOffset(e.first, e.second.first);
		}
	}
	for (auto e : _request.speeds) {
		if (e.second.second && (headIDs.find(e.first) != headIDs.end())) {
			setSpeed(e.first, e.second.first);
		}
	}
	for (auto e : _request.torques) {
		if (e.second.second && (headIDs.find(e.first) != headIDs.end())) {
			setTorque(e.first, e.second.first);
		}
	}
}

