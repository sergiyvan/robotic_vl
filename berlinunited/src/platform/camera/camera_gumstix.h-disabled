/** @file
 **
 ** Class for the Gumstix camera board used in RoboCup 2009 on Gumstix Verdex Pro
 */

#ifndef CAMERA_GUMSTIX_H_
#define CAMERA_GUMSTIX_H_

#include "camera_v4l2.h"

class CameraGumstix : public CameraV4L2 {
public:
	CameraGumstix();
	virtual ~CameraGumstix() {}

	virtual std::string getCameraName() {
		return "Bennet's Gumstix Camera board";
	}

	virtual bool openCamera(const char* deviceName, uint16_t requestedImageWidth=640, uint16_t requestedImageHeight=480);

	virtual void setSetting(CAMERA_SETTING setting, int32_t value);
//	virtual int32_t getSetting(CAMERA_SETTING setting);

	void setRegister(unsigned char reg, unsigned char val);

protected:
	virtual uint32_t getPixelFormat();
	virtual void defaultConfiguration();
};

#endif /* CAMERA_GUMSTIX_H_ */
