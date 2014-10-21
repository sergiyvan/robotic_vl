#include "motions.h"

#include "debug.h"

Motions::~Motions() {
	for (auto& e : staticMotions) {
		delete e.second;
	}
}
void Motions::addMotion(MotionType motiontype, const char* motiondata, uint32_t motionsize) {

	de::fumanoids::message::RobotMotion *motion = new de::fumanoids::message::RobotMotion;
	if (motion->ParseFromArray(motiondata, motionsize)) {
		if (hasMotion(motiontype)) {
			ERROR("Adding an already existing motion!");
			staticMotions[motiontype]->CopyFrom(*motion);
		} else {
			staticMotions[motiontype] = motion;
		}
	} else {
		delete motion;
		ERROR("Could not load motion " STRINGIFY(motiondata) " (" STRINGIFY(motiontype) ")");
	}
}

void Motions::addMotion(MotionType motionType, const de::fumanoids::message::RobotMotion &motion) {
	if (hasMotion(motionType)) {
		ERROR("Adding an already existing motion!");
		staticMotions[motionType]->CopyFrom(motion);
		return;
	}

	de::fumanoids::message::RobotMotion *motionCopy = new de::fumanoids::message::RobotMotion;
	motionCopy->CopyFrom(motion);
	staticMotions[motionType] = motionCopy;
}

void Motions::replaceMotion(MotionType motionType, const de::fumanoids::message::RobotMotion &motion) {
	if (hasMotion(motionType)) {
		staticMotions[motionType]->CopyFrom(motion);
	} else {
		de::fumanoids::message::RobotMotion *motionCopy = new de::fumanoids::message::RobotMotion;
		motionCopy->CopyFrom(motion);
		staticMotions[motionType] = motionCopy;
	}
}

bool Motions::hasMotion(MotionType motionType) const {
	return staticMotions.find(motionType) != staticMotions.end();
}

de::fumanoids::message::RobotMotion* Motions::getMotion(MotionType motionType) const {
	if (false == hasMotion(motionType))
		return nullptr;

	return staticMotions.at(motionType);
}

