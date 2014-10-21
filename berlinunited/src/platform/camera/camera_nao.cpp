#include "camera_nao.h"
#include "platform/image/image.h"
#include "platform/system/timer.h"

#include <linux/videodev2.h>

REGISTER_CAMERA("nao", CameraNao, "Aldebaran Nao v4 USB cam via v4l2");

CameraNao::CameraNao() {

	/* Further possible options (copied from driver):
	V4L2_CID_HUE: 0, -180, 180
	V4L2_CID_VFLIP:
	V4L2_CID_HFLIP: 0, 0, 1
	V4L2_CID_SHARPNESS: 0, -7, 7
	V4L2_CID_EXPOSURE: 0, 0, 512
	V4L2_CID_DO_WHITE_BALANCE: -166, -180, 180
	V4L2_CID_BACKLIGHT_COMPENSATION: 1, 0, 4 */

	addSupportedSetting( CAMERA_BRIGHTNESS,         V4L2_CID_BRIGHTNESS,                 "Brightness",                     55,    0,   255 );
	addSupportedSetting( CAMERA_CONTRAST,           V4L2_CID_CONTRAST,                   "Contrast",                       32,    0,   127 );
	addSupportedSetting( CAMERA_SATURATION,         V4L2_CID_SATURATION,                 "Saturation",                    128,    0,   255 );
	addSupportedSetting( CAMERA_GAIN,               V4L2_CID_GAIN,                       "Gain",                           32,    0,   255 );
	addSupportedSetting( CAMERA_SHARPNESS,          V4L2_CID_SHARPNESS,                  "Sharpness",                       0,   -7,     7 );
	addSupportedSetting( CAMERA_AUTO_EXPOSURE,      V4L2_CID_EXPOSURE_AUTO,              "Auto-Exposure (1=off)",           1,    0,     1 );
	addSupportedSetting( CAMERA_AUTO_WHITE_BALANCE, V4L2_CID_AUTO_WHITE_BALANCE,         "Auto White Balance",              1,    0,     1 );
}

