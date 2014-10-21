#ifndef MOTIONREQUEST_H__
#define MOTIONREQUEST_H__

#include "motionType.h"
#include "utils/math/Math.h"

#include "representations/motion/walkerTargetValues.h"

/*------------------------------------------------------------------------------------------------*/

typedef enum {
	  KICK_WITH_LEFT
	, KICK_WITH_RIGHT
	, NO_KICK
} KickLeg;

/** Parameters for the walker
 */

class WalkerParam {
public:
	WalkerParam()
		: forwardSpeed(0)
		, sidewardSpeed(0)
		, rotationSpeed(0)
		, footDrift(0)
		, noArmMovements(false)
		, kickSide(NO_KICK)
		, orthoKickSide(NO_KICK)
		, currentTargetConfiguration({0.0, 0.0, 0.0})
	{}
	virtual ~WalkerParam() {}


	inline void setSpeeds(float fwd, float swd, float rot) {
		forwardSpeed  = Math::limited(fwd, -100.0, 100.0);
		sidewardSpeed = Math::limited(swd, -100.0, 100.0);
		rotationSpeed = Math::limited(rot, -100.0, 100.0);
	}
	void setForwardSpeed(float _speed)  { forwardSpeed  = Math::limited(_speed, -100.0, 100.0); }
	void setSidewardSpeed(float _speed) { sidewardSpeed = Math::limited(_speed, -100.0, 100.0); }
	void setRotationSpeed(float _speed) { rotationSpeed = Math::limited(_speed, -100.0, 100.0); }
	void setFootDrift(float _drift) { footDrift = _drift; }
	void setKickSide(KickLeg _leg) { kickSide = _leg; }
	void setOrthoKickSide(KickLeg leg) { orthoKickSide = leg; }
	void setCurrentTargetConfiguration(WalkerConfiguration targetConfiguration) { currentTargetConfiguration = targetConfiguration; }

	float getForwardSpeed() const { return forwardSpeed; }
	float getSidewardSpeed() const { return sidewardSpeed; }
	float getRotationSpeed() const { return rotationSpeed; }
	float getFootDrift() const { return footDrift; }
	bool getNoArmMovements() const { return noArmMovements; }
	KickLeg getKickSide() const { return kickSide; }
	KickLeg getOrthoKickSide() const { return kickSide; }
	WalkerConfiguration getCurrentTargetConfiguration() const { return currentTargetConfiguration; }


	void setTargetConfiguration() {}


private:
	// TODO: proper units
	float   forwardSpeed;
	float   sidewardSpeed;
	float   rotationSpeed;

	float   footDrift;

	bool      noArmMovements;
	KickLeg   kickSide;
	KickLeg   orthoKickSide;

	WalkerConfiguration currentTargetConfiguration;


};


/*------------------------------------------------------------------------------------------------*/

/** Parameters for the kicker
 **
 */

class KickParam {
public:
	KickParam()
		: leg(KICK_WITH_LEFT)
		, kickIntensity (100.0)
		, kickDirection (0.0)
	{}

	KickLeg   leg;
	double    kickIntensity; // 0 - 100 %
	double    kickDirection; // negative: right
//	int16_t   distance; //< attempted distance to shoot
};


/*------------------------------------------------------------------------------------------------*/

/** The motion request representation contains information
 ** about which motion to play next.
 */

class MotionRequest {
public:
	MotionRequest()
		: motion(MOTION_NONE)
		, uninterruptable(false)
	{}

	virtual ~MotionRequest() {}

	MotionType    motion;
	bool          uninterruptable;

	WalkerParam   walkerParam;
	KickParam     kickParam;
};


#endif
