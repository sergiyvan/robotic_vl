#include "camera_gumstix.h"

#include <linux/videodev2.h>

#define V4L2_CID_R_GAIN     V4L2_CID_PRIVATE_BASE + 1
#define V4L2_CID_G_GAIN     V4L2_CID_PRIVATE_BASE + 2
#define V4L2_CID_B_GAIN     V4L2_CID_PRIVATE_BASE + 3
#define V4L2_DIRTYREGSET    V4L2_CID_PRIVATE_BASE + 4


/*------------------------------------------------------------------------------------------------*/

/**
 */

CameraGumstix::CameraGumstix() {
#ifdef IMAGEFORMAT_YUV422
	addSupportedSetting( CAMERA_BRIGHTNESS,         V4L2_CID_BRIGHTNESS,         "Brightness",               0, 0,      255 );
	addSupportedSetting( CAMERA_CONTRAST,           V4L2_CID_CONTRAST,           "Contrast",               128, 1,      255 );
	addSupportedSetting( CAMERA_SATURATION,         V4L2_CID_SATURATION,         "Saturation",             128, 0,      255 );
#endif
	addSupportedSetting( CAMERA_EXPOSURE,           V4L2_CID_EXPOSURE,           "Exposure",           0x27100, 0, 0xffffff );
	addSupportedSetting( CAMERA_GAIN,               V4L2_CID_GAIN,               "Gain",                    32, 0,      255 );
	addSupportedSetting( CAMERA_R_GAIN,             V4L2_CID_R_GAIN,             "R Gain",                  24, 0,       63 );
	addSupportedSetting( CAMERA_G_GAIN,             V4L2_CID_G_GAIN,             "G Gain",                  24, 0,       63 );
	addSupportedSetting( CAMERA_B_GAIN,             V4L2_CID_B_GAIN,             "B Gain",                  24, 0,       63 );
#ifdef IMAGEFORMAT_YUV422
	addSupportedSetting( CAMERA_HUE,                V4L2_CID_HUE,                "Hue",                 0x8000, 0,   0xffff );
#endif
	addSupportedSetting( CAMERA_AUTO_WHITE_BALANCE, V4L2_CID_AUTO_WHITE_BALANCE, "Auto White Balance",       1, 0,        1 );
	addSupportedSetting( CAMERA_AUTO_EXPOSURE,      V4L2_CID_EXPOSURE_AUTO,      "Auto-Exposure",            1, 0,        1 );
	addSupportedSetting( CAMERA_GAMMA,              0,                           "Gamma",                    1, 0,        1 );
	addSupportedSetting( CAMERA_HFLIP_IMAGE,        V4L2_CID_HFLIP,              "Horizontal Flip",          0, 0,        1 );
	addSupportedSetting( CAMERA_REGISTER,           V4L2_DIRTYREGSET,            "Register (Danger!)",       0, 0,   0xffff );
}



/*------------------------------------------------------------------------------------------------*/

/** Get pixel format.
 **
 ** @return pixel format to use
 */

uint32_t CameraGumstix::getPixelFormat() {
#ifdef IMAGEFORMAT_YUV422
	return V4L2_PIX_FMT_YUYV;
#else
	return V4L2_PIX_FMT_SBGGR8;
#endif
}


/*------------------------------------------------------------------------------------------------*/

/** Open the camera.
 **
 ** The Gumstix camera has two lenses and only sends them stitched together as an 1280x480 image.
 **
 ** @param deviceName
 ** @param requestedImageWidth
 ** @param requestedImageHeight
 **
 */

bool CameraGumstix::openCamera(const char* deviceName, uint16_t requestedImageWidth, uint16_t requestedImageHeight) {
	if (requestedImageWidth != 640 || requestedImageHeight != 480)
		return false;

	return CameraV4L2::openCamera(deviceName, 640, 480);
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

void CameraGumstix::defaultConfiguration() {
	setSetting(CAMERA_HFLIP_IMAGE,            0);
	setSetting(CAMERA_AUTO_EXPOSURE,          0);
	setSetting(CAMERA_AUTO_WHITE_BALANCE,     0);
	setSetting(CAMERA_GAIN,                  42);
	setSetting(CAMERA_EXPOSURE,           85000);
	setSetting(CAMERA_HUE,                33000);
	setSetting(CAMERA_SATURATION,           140);
	setSetting(CAMERA_R_GAIN,                24);
	setSetting(CAMERA_G_GAIN,                24);
	setSetting(CAMERA_B_GAIN,                24);
	setSetting(CAMERA_CONTRAST,             128);
	setSetting(CAMERA_BRIGHTNESS,             0);
	setSetting(CAMERA_GAMMA,                  1);

#ifndef IMAGEFORMAT_YUV422
	// set register 3 (SENSOR_CONTROL_3) to Bayer mode
	setRegister(3, uint8_t(1<<7) + 1);

	// setRegister(0x31, (uint8_t)(1<<5));

	// offset row so we get the Bayer data in the correct format
	setRegister(0x0b, 3);
#endif
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

void CameraGumstix::setSetting(CAMERA_SETTING setting, int32_t value) {
	if (setting == CAMERA_GAMMA)
		setRegister(48, value > 0 ? 2 : 0);
	else
		return CameraV4L2::setSetting(setting, value);
}


/*------------------------------------------------------------------------------------------------*/

/**
 */
/*
int32_t CameraGumstix::getSetting(CAMERA_SETTING setting) {
	if (setting == CAMERA_GAMMA)
		return getRegister(48);
	else
		return CameraV4L2::getSetting(setting);
}
*/

/*------------------------------------------------------------------------------------------------*/

/**
 */

void CameraGumstix::setRegister(unsigned char reg, unsigned char val) {
	setSetting(CAMERA_REGISTER, (reg << 8) | val);
/*
	if (reg == 48) {
		if(val == 0) {
			INFO("SET GAMMA - CORRECTION OFF!");
		} else if(val == 2) {
			INFO("Set GAMMA - CORRECTION ON!");
		}
		this->gammaValue = val;
	}
*/
}
