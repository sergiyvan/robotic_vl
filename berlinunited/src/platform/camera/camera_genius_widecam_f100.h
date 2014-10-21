/** @file
 **
 ** Class for the Logitech C910
 */

#ifndef CAMERA_GENIUS_WIDECAM_F100_H_
#define CAMERA_GENIUS_WIDECAM_F100_H_

#include "camera_v4l2.h"
#include <string>

class CameraGeniusWideCamF100 : public CameraV4L2 {
public:
	CameraGeniusWideCamF100();
	virtual ~CameraGeniusWideCamF100() {}

	virtual std::string getCameraName() {
		return "Genius WideCam F100";
	}
};

#endif
