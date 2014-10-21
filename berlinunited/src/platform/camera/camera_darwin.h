/** @file
 **
 ** Class for the Logitech C905 built into the DARwIn OP
 */

#ifndef CAMERA_DARWIN_H_
#define CAMERA_DARWIN_H_

#include "platform/camera/camera_v4l2.h"
#include <string>

class CameraDARwIn : public CameraV4L2 {
public:
	CameraDARwIn();
	virtual ~CameraDARwIn() {}
	virtual CameraImage* getImage() {
		return image;
	}

	virtual bool capture();

	virtual std::string getCameraName() {
		return "Logitech C905 (DARwIn)";
	}
};

#endif
