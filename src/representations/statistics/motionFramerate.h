#ifndef MOTION_FRAMERATE_H_
#define MOTION_FRAMERATE_H_

#include "utils/units.h"
class MotionFramerate {
public:
	MotionFramerate() : latest_fps(0*hertz), total_fps(0*hertz), framecounter(0) {}

	Hertz latest_fps; /// average fps over the last few seconds
	Hertz total_fps;  /// average fps since start of execution

	uint32_t framecounter;
};

#endif
