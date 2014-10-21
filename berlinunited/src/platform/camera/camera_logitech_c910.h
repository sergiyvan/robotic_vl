/** @file
 **
 ** Class for the Logitech C910
 */

#ifndef CAMERA_LOGITECH_C910_H_
#define CAMERA_LOGITECH_C910_H_

#include "camera_v4l2.h"
#include <string>

class CameraLogitechC910 : public CameraV4L2 {
public:
	CameraLogitechC910();
	virtual ~CameraLogitechC910() {}

	virtual std::string getCameraName() {
		return "Logitech C910";
	}
};

#endif
