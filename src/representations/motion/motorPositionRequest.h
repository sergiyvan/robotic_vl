#ifndef REPRESENTATION_MOTORPOSITIONREQUEST
#define REPRESENTATION_MOTORPOSITIONREQUEST

#include <array>

#include "ModuleFramework/Serializer.h"
#include "platform/hardware/robot/motorIDs.h"
#include "platform/system/timer.h"
#include "utils/units.h"

class MotorPositionRequest {
	std::map<MotorID, std::pair<Degree, int>>  positions; // Indicates target position of motor
	std::map<MotorID, std::pair<Degree, bool>> offsets;   // Indicates the offset of this motor
	std::map<MotorID, std::pair<RPM, bool>>    speeds;    // Indicates speed of motor
	std::map<MotorID, std::pair<double, bool>> forces;    // Indicates torque of motor
	std::map<MotorID, std::pair<bool, bool>>   torques;   // Indicates if torque should be enabled

public:
	/** this will clear the request, so double request can be detected*/
	void clear(bool _totalClear=false);

	void setPosition(MotorID _id, Degree _angle);
	void setOffset(MotorID _id, Degree _angleOffset);
	void setSpeed(MotorID _id, RPM _speed);
	void setPositionAndSpeed(MotorID _id, Degree _angle, RPM _speed);
	void setForce(MotorID _id, double force);
	void setTorque(MotorID _id, bool _on);
	void setAllTorques(bool _on);

	Degree getPosition(MotorID _id) const;
	Degree getOffset(MotorID _id) const;
	RPM getSpeed(MotorID _id) const;

	std::map<MotorID, Degree> getPositionRequests() const;
	std::map<MotorID, Degree> getOffsetRequests() const;
	std::map<MotorID, RPM>    getSpeedRequests() const;
	std::map<MotorID, double> getForceRequests() const;
	std::map<MotorID, bool>   getTorqueRequests() const;

	// merges the request in _request into this representation
	// This is for cognition -> motion exchange
	void merge(const MotorPositionRequest& _request);
	void mergeHeadOnly(const MotorPositionRequest& _request);

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & positions;
		ar & offsets;
		ar & speeds;
		ar & torques;
	}
};

REGISTER_SERIALIZATION(MotorPositionRequest, 1)


#endif

