#include "robotDescription.h"
#include "platform/hardware/robot/motorIDs.h"
#include "debugging/debug3d.h"
#include "utils/utils.h"

#include "services.h"
#include "platform/hardware/robot/robotModel.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/optional/optional.hpp>

#include <tools/kinematicEngine/kinematicNodeFactory.h>


// for compatibility reasons, define common IDs globally

MotorID MOTOR_HEAD_PITCH           = MOTOR_NONE;
MotorID MOTOR_HEAD_TURN            = MOTOR_NONE;

//arms
MotorID MOTOR_LEFT_ARM_PITCH        = MOTOR_NONE;
MotorID MOTOR_RIGHT_ARM_PITCH       = MOTOR_NONE;
MotorID MOTOR_LEFT_ARM_ROLL         = MOTOR_NONE;
MotorID MOTOR_RIGHT_ARM_ROLL        = MOTOR_NONE;
MotorID MOTOR_LEFT_ELBOW            = MOTOR_NONE;
MotorID MOTOR_RIGHT_ELBOW           = MOTOR_NONE;

// body center
MotorID MOTOR_STOMACH               = MOTOR_NONE;
MotorID MOTOR_SPINE                 = MOTOR_NONE;

// legs
MotorID MOTOR_LEFT_HIP_ROLL         = MOTOR_NONE;
MotorID MOTOR_RIGHT_HIP_ROLL        = MOTOR_NONE;
MotorID MOTOR_LEFT_KNEE_TOP         = MOTOR_NONE;
MotorID MOTOR_RIGHT_KNEE_TOP        = MOTOR_NONE;
MotorID MOTOR_LEFT_KNEE_BOTTOM      = MOTOR_NONE;
MotorID MOTOR_RIGHT_KNEE_BOTTOM     = MOTOR_NONE;
MotorID MOTOR_LEFT_FOOT_ROLL        = MOTOR_NONE;
MotorID MOTOR_RIGHT_FOOT_ROLL       = MOTOR_NONE;
MotorID MOTOR_LEFT_FOOT_YAW         = MOTOR_NONE;
MotorID MOTOR_RIGHT_FOOT_YAW        = MOTOR_NONE;

// end effectors
MotorID EFFECTOR_ID_ROOT                    = MOTOR_NONE;
MotorID EFFECTOR_ID_GYROSCOPE               = MOTOR_NONE;
MotorID EFFECTOR_ID_CAMERA_POSITION         = MOTOR_NONE;
MotorID EFFECTOR_ID_FOOT_LEFT_FRONT_CENTER  = MOTOR_NONE;
MotorID EFFECTOR_ID_FOOT_RIGHT_FRONT_CENTER = MOTOR_NONE;


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

RobotDescription::RobotDescription(std::string robotDescriptionPath) {
	// build the kinematic tree
	generateFromXML(robotDescriptionPath);

	// collect motor IDs and create offset configuration options
	for (const auto &it : m_nodes) {
		if (it.second->isServo()) {
			motorIDs.insert(it.first);

			std::string motorName = it.second->getName();
			if (motorName != "Unknown") {
				std::string configName  = std::string("motors.offset.") + motorName;
				std::string description = std::string("Offset for motor ") + motorName;
				services.getConfig().registerOption<Degree>(configName, 0*degrees, description);
			}
		}
	}

	// push the added configuration options to the active configuration
//FIXME!!!!
//	ConfigRegistry::getInstance().applyDefaultValuesTo(&services.getConfig());


	MOTOR_HEAD_PITCH            = getEffectorID("HeadPitch");
	MOTOR_HEAD_TURN             = getEffectorID("HeadYaw");

	//arms
	MOTOR_LEFT_ARM_PITCH        = getEffectorID("LeftArmPitch");
	MOTOR_RIGHT_ARM_PITCH       = getEffectorID("RightArmPitch");
	MOTOR_LEFT_ARM_ROLL         = getEffectorID("LeftArmRoll");
	MOTOR_RIGHT_ARM_ROLL        = getEffectorID("RightArmRoll");
	MOTOR_LEFT_ELBOW            = getEffectorID("LeftElbow");
	MOTOR_RIGHT_ELBOW           = getEffectorID("RightElbow");

	// body center
	MOTOR_STOMACH               = getEffectorID("Stomach");
	MOTOR_SPINE                 = getEffectorID("Spine");

	// legs
	MOTOR_LEFT_HIP_ROLL         = getEffectorID("LeftHipRoll");
	MOTOR_RIGHT_HIP_ROLL        = getEffectorID("RightHipRoll");
	MOTOR_LEFT_KNEE_TOP         = getEffectorID("LeftKneeTop");
	MOTOR_RIGHT_KNEE_TOP        = getEffectorID("RightKneeTop");
	MOTOR_LEFT_KNEE_BOTTOM      = getEffectorID("LeftKneeBottom");
	MOTOR_RIGHT_KNEE_BOTTOM     = getEffectorID("RightKneeBottom");
	MOTOR_LEFT_FOOT_ROLL        = getEffectorID("LeftFootRoll");
	MOTOR_RIGHT_FOOT_ROLL       = getEffectorID("RightFootRoll");
	MOTOR_LEFT_FOOT_YAW         = getEffectorID("LeftFootYaw");
	MOTOR_RIGHT_FOOT_YAW        = getEffectorID("RightFootYaw");

	EFFECTOR_ID_ROOT                    = getEffectorID("root");
	EFFECTOR_ID_GYROSCOPE               = getEffectorID("gyroscope");
	EFFECTOR_ID_CAMERA_POSITION         = getEffectorID("CameraPosition");
	EFFECTOR_ID_FOOT_LEFT_FRONT_CENTER  = getEffectorID("footLeftFrontCenter");
	EFFECTOR_ID_FOOT_RIGHT_FRONT_CENTER = getEffectorID("footRightFrontCenter");
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

RobotDescription::~RobotDescription() {
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void RobotDescription::generateFromXML(std::string robotDescriptionPath)
{
	if (false == fileExists(robotDescriptionPath)) {
		ERROR("Could not find robot description file ./%s. Did you copy it to the robot?", robotDescriptionPath.c_str());
		return;
	}

	boost::property_tree::ptree tree;
	boost::property_tree::read_xml(robotDescriptionPath.c_str(), tree);

	int robotDescriptionNodeCnt = tree.count("robotdescription");

	KinematicNodeFactory nodeFactory;

	if (0 < robotDescriptionNodeCnt) {
		BOOST_FOREACH(boost::property_tree::ptree::value_type const &child, tree.get_child("robotdescription") ) {
			boost::optional<std::string> name = child.second.get_optional<std::string>("<xmlattr>.name");
			if (name.is_initialized() && name.get() == "root") {
				KinematicNode *rootNode = nodeFactory.createNodeFromPTree(child);
				m_nodes[rootNode->getID()] = rootNode;
				buildFromPTree(child.second, rootNode);
			}
		}
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void RobotDescription::buildFromPTree(boost::property_tree::ptree subTree, KinematicNode *parent) {
	KinematicNodeFactory nodeFactory;
	try {
		BOOST_FOREACH(boost::property_tree::ptree::value_type const &child, subTree) {
			if (child.first == "effector") {
				KinematicNode *node = nodeFactory.createNodeFromPTree(child);
				node->setParent(parent);
				m_nodes[node->getID()] = node;
				buildFromPTree(child.second, node);
			}
		}
	} catch (const boost::property_tree::xml_parser::xml_parser_error& ex) {
		ERROR("Error in file %s at line %d: %s", ex.filename().c_str(), (int)ex.line(), ex.what());
	}
}
