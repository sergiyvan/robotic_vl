#include "camera_philips_spc630nc.h"

#include <linux/videodev2.h>


/*------------------------------------------------------------------------------------------------*/

/**
 */

CameraPhilipsSPC::CameraPhilipsSPC() {
	addSupportedSetting( CAMERA_BRIGHTNESS,         V4L2_CID_BRIGHTNESS,                "Brightness",            24,     0,     37 );
	addSupportedSetting( CAMERA_CONTRAST,           V4L2_CID_CONTRAST,                  "Contrast",             124,     0,    200 );
	addSupportedSetting( CAMERA_SATURATION,         V4L2_CID_SATURATION,                "Saturation",           121,     0,    200 );
	addSupportedSetting( CAMERA_GAMMA,              V4L2_CID_GAMMA,                     "Gamma",                 18,     1,     31 );
	addSupportedSetting( CAMERA_SHARPNESS,          V4L2_CID_SHARPNESS,                 "Sharpness",             15,     0,     63 );
	addSupportedSetting( CAMERA_AUTO_WHITE_BALANCE, V4L2_CID_AUTO_WHITE_BALANCE,        "Auto White Balance",     1,     0,      1 );
	addSupportedSetting( CAMERA_WHITE_BALANCE_TEMP, V4L2_CID_WHITE_BALANCE_TEMPERATURE, "White Balance Temp",  6600,  2800,   6800 );
	addSupportedSetting( CAMERA_BACKLIGHT_COMP,     V4L2_CID_BACKLIGHT_COMPENSATION,    "Backlight Comp.",        0,     0,      2 );
}
