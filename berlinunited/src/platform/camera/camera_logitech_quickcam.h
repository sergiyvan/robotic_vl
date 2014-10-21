/** @file
 **
 ** Class for the Logitech Quickcam (pro for notebooks | 9000)
 */

#ifndef CAMERA_LOGITECH_QUICKCAM_H_
#define CAMERA_LOGITECH_QUICKCAM_H_

#include "camera_v4l2.h"
#include <string>

class CameraLogitechQuickcam : public CameraV4L2 {
public:
	CameraLogitechQuickcam();
	virtual ~CameraLogitechQuickcam() {}

	virtual std::string getCameraName() {
		return "Logitech Quickcam";
	}
};

#endif
