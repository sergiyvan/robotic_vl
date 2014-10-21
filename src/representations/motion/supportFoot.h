#ifndef REPRESENTATION_SUPPORTFOOT_H
#define REPRESENTATION_SUPPORTFOOT_H

/**
 * This class Provides Information about the current support foot.
 * The current support foot is the foot that is currently on the ground.
 */
class SupportFoot {
public:
	enum Foot { NONE = 0, LEFT = 1, RIGHT = 2, BOTH = 3};

	SupportFoot() : foot(BOTH) {}
	SupportFoot(Foot _foot) : foot(_foot) {}

	bool isSupportFoot(Foot _foot) const {
		return (foot & _foot) == _foot;
	}

	Foot getSupportFoot() const {
		return foot;
	}

	void setSupportFoot(Foot _foot) {
		foot = _foot;
	}
private:
	Foot foot;

};
#endif

