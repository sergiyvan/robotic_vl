/*
 * imu.h
 *
 *  Created on: June 06, 2014
 *      Author: sgg
 */

#ifndef MOTIONS_H
#define MOTIONS_H

#include "messages/msg_motion.pb.h"
#include "representations/motion/motionType.h"


class Motions {
public:
	virtual ~Motions();

	virtual bool init() { return true; }

	virtual bool hasMotion(MotionType motionType) const;
	virtual de::fumanoids::message::RobotMotion* getMotion(MotionType motionType) const;

	virtual void addMotion(MotionType motiontype, const de::fumanoids::message::RobotMotion& motion);
	virtual void replaceMotion(MotionType motionType, const de::fumanoids::message::RobotMotion& motion);

protected:
	virtual void addMotion(MotionType motiontype, const char* motiondata, uint32_t motionsize);

private:
	std::map<MotionType, de::fumanoids::message::RobotMotion*> staticMotions;
};

#endif

