#ifndef ROBOTDESCRIPTION_H__
#define ROBOTDESCRIPTION_H__

#include "tools/kinematicEngine/kinematicNode.h"

#include <boost/property_tree/ptree.hpp>

#include <string>
#include <map>
#include <set>

/**
 ** \class RobotDescription
 ** \brief Description of the current robot, based on the robotDescription.xml
 */

class RobotDescription {
public:
	RobotDescription(std::string robotDescriptionPath);
	virtual ~RobotDescription();

	inline const std::map<MotorID, KinematicNode*> getNodes() const {
		return m_nodes;
	}

	inline const std::set<MotorID>& getMotorIDs() const {
		return motorIDs;
	}

	const std::string getMotorName(MotorID id) const {
		const auto &it = m_nodes.find(id);
		if (it != m_nodes.end()) {
			assert(it->second->isServo());
			return it->second->getName();
		} else {
			return "Unknown";
		}
	}

	MotorID getEffectorID(const std::string &effectorName) const {
		for (const auto &it : m_nodes) {
			if (it.second->getName() == effectorName) {
				return it.first;
			}
		}

		return MOTOR_NONE;
	}

	const std::set<MotorID>& getEffectorIDs() const {
		static std::set<MotorID> ids;
		return ids;
	}


private:
	std::map<MotorID, KinematicNode*> m_nodes;
	std::set<MotorID> motorIDs;

	/**
	 * generate the kinematic tree from a given xml file (robot description)
	 * @param path path to file
	 */
	void generateFromXML(std::string path);

	void buildFromPTree(boost::property_tree::ptree subTree, KinematicNode *parent);
};


extern MotorID MOTOR_HEAD_PITCH;
extern MotorID MOTOR_HEAD_TURN;

//arms
extern MotorID MOTOR_LEFT_ARM_PITCH;
extern MotorID MOTOR_RIGHT_ARM_PITCH;
extern MotorID MOTOR_LEFT_ARM_ROLL;
extern MotorID MOTOR_RIGHT_ARM_ROLL;
extern MotorID MOTOR_LEFT_ELBOW;
extern MotorID MOTOR_RIGHT_ELBOW;

// body center
extern MotorID MOTOR_STOMACH;
extern MotorID MOTOR_SPINE;

// legs
extern MotorID MOTOR_LEFT_HIP_ROLL;
extern MotorID MOTOR_RIGHT_HIP_ROLL;
extern MotorID MOTOR_LEFT_KNEE_TOP;
extern MotorID MOTOR_RIGHT_KNEE_TOP;
extern MotorID MOTOR_LEFT_KNEE_BOTTOM;
extern MotorID MOTOR_RIGHT_KNEE_BOTTOM;
extern MotorID MOTOR_LEFT_FOOT_ROLL;
extern MotorID MOTOR_RIGHT_FOOT_ROLL;
extern MotorID MOTOR_LEFT_FOOT_YAW;
extern MotorID MOTOR_RIGHT_FOOT_YAW;

extern MotorID EFFECTOR_ID_ROOT;
extern MotorID EFFECTOR_ID_GYROSCOPE;
extern MotorID EFFECTOR_ID_CAMERA_POSITION;

extern MotorID EFFECTOR_ID_FOOT_LEFT_FRONT_CENTER;
extern MotorID EFFECTOR_ID_FOOT_RIGHT_FRONT_CENTER;


#endif
