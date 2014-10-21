/** @file
 **
 ** Class for the Logitech C930e
 */

#ifndef CAMERA_LOGITECH_C930E_H_
#define CAMERA_LOGITECH_C930E_H_

#include "camera_v4l2.h"
#include <string>

class CameraLogitechC930e : public CameraV4L2 {
public:
	CameraLogitechC930e();
	virtual ~CameraLogitechC930e() {}

	virtual std::string getCameraName() {
		return "Logitech C930e";
	}
};

#endif
