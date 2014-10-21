#ifndef VISION_FRAMERATE_H_
#define VISION_FRAMERATE_H_

#include "utils/units.h"

class VisionFramerate {
public:
	VisionFramerate() : latest_fps(0*hertz), total_fps(0*hertz) {}

	Hertz latest_fps; /// average fps over the last few seconds
	Hertz total_fps;  /// average fps since start of execution
};

#endif


