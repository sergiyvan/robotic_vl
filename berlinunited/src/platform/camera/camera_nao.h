/** @file
 **
 ** Class for the NAOs Aptina MT9M114 camera
 */

#ifndef CAMERA_NAO_H_
#define CAMERA_NAO_H_

#include "camera_v4l2.h"
#include <string>

class CameraNao : public CameraV4L2 {
public:
	CameraNao();
	virtual ~CameraNao() {}

	virtual std::string getCameraName() {
		return "NAOs Aptina MT9M114";
	}
};

#endif
