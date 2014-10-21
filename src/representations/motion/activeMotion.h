#ifndef ACTIVEMOTION_H__
#define ACTIVEMOTION_H__

#include "motionType.h"

#include <ModuleFramework/Serializer.h>


/*------------------------------------------------------------------------------------------------*/
/**
 * @brief Information about what the robot is currently doing.
 *
 * It is NO MotionRequest.
 */
class ActiveMotion {
public:
	ActiveMotion()
		: motion(MOTION_NONE)
		, stopWhenStable(false)
		, motionInit(false)
	{}

	virtual ~ActiveMotion() {}

	MotionType motion;
	bool       stopWhenStable;
	bool       motionInit;

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & motion;
		ar & stopWhenStable;
		ar & motionInit;
	}
};

REGISTER_SERIALIZATION(ActiveMotion, 1)

#endif // ACTIVEMOTION_H__
