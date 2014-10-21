#ifndef MOTIONSTATUS_H__
#define MOTIONSTATUS_H__

#include <ModuleFramework/Serializer.h>


class MotionStatus {
public:
	MotionStatus()
		: robotIsStable(true)      // we do not know any better, so let's assume this
		, motionHasFinished(true)  // we do not know any better, so let's assume this
		, cognitionInputEnabled(true)
		, allowHeadMovements(true)
	{
	}

	virtual ~MotionStatus() {
	}

	bool robotIsStable;     ///< set when motion has achieved a stable position
	bool motionHasFinished; ///< set when motion has finished playing

	// control options
	bool cognitionInputEnabled;

	/// whether the head is allowed to move
	bool allowHeadMovements;

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & robotIsStable;
		ar & motionHasFinished;
		ar & cognitionInputEnabled;
		ar & allowHeadMovements;
	}
};

REGISTER_SERIALIZATION(MotionStatus, 1)

#endif

