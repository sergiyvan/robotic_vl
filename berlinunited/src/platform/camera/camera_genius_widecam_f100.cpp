#include "camera_genius_widecam_f100.h"
#include "platform/image/image.h"
#include "platform/system/timer.h"

#include <linux/videodev2.h>


REGISTER_CAMERA("f100", CameraGeniusWideCamF100, "Genius WideCam F100 USB webcam via v4l2");


/*------------------------------------------------------------------------------------------------*/

/** The following applies to the Genius WideCam F100 camera with UVC interface.
 **
 ** Auto-Exposure == 1: Manual mode
 ** Auto-Exposure == 3: Aperture priority mode, camera adjusts exposure time automatically to
 **                     handle different lighting situations (default value)
 ** Control Brightness                          (code: 980900, val: 0, min: -64, max: 64)
 ** Control Contrast                            (code: 980901, val: 32, min: 0, max: 95)
 ** Control Saturation                          (code: 980902, val: 55, min: 0, max: 100)
 ** Control Hue                                 (code: 980903, val: 0, min: -2000, max: 2000)
 ** Control White Balance Temperature, Auto     (code: 98090c, val: 1, min: 0, max: 1)
 ** Control Gamma                               (code: 980910, val: 165, min: 100, max: 300)
 ** Control Power Line Frequency                (code: 980918, val: 1, min: 0, max: 2)
 **  Menu items:
 **  Disabled
 **  50 Hz
 **  60 Hz
 ** Control White Balance Temperature           (code: 98091a, val: 4600, min: 2800, max: 6500)
 ** Control Sharpness                           (code: 98091b, val: 2, min: 1, max: 7)
 ** Control Backlight Compensation              (code: 98091c, val: 0, min: 0, max: 1)
 ** Control Exposure, Auto                      (code: 9a0901, val: 3, min: 0, max: 3)
 **  Menu items:
 **  Manual Mode (1)
 **  Aperture Priority Mode (3)
 ** Control Exposure (Absolute)                 (code: 9a0902, val: 166, min: 50, max: 10000)
 */

CameraGeniusWideCamF100::CameraGeniusWideCamF100() {
	addSupportedSetting( CAMERA_BRIGHTNESS,         V4L2_CID_BRIGHTNESS,                 "Brightness",                       0,   -64,     64 );
	addSupportedSetting( CAMERA_CONTRAST,           V4L2_CID_CONTRAST,                   "Contrast",                        32,     0,     95 );
	addSupportedSetting( CAMERA_SATURATION,         V4L2_CID_SATURATION,                 "Saturation",                      55,     0,    100 );
	addSupportedSetting( CAMERA_HUE,                V4L2_CID_HUE,                        "Hue",                              0, -2000,   2000 );
	addSupportedSetting( CAMERA_SHARPNESS,          V4L2_CID_SHARPNESS,                  "Sharpness",                        2,     1,      7 );
	addSupportedSetting( CAMERA_AUTO_EXPOSURE,      V4L2_CID_EXPOSURE_AUTO,              "Auto-Exposure (1=off)",            1,     1,      3 );
	addSupportedSetting( CAMERA_EXPOSURE,           V4L2_CID_EXPOSURE_ABSOLUTE,          "Exposure Absolute",              160,    50,  10000 );
	addSupportedSetting( CAMERA_AUTO_WHITE_BALANCE, V4L2_CID_AUTO_WHITE_BALANCE,         "Auto White Balance",               0,     0,      1 );
	addSupportedSetting( CAMERA_WHITE_BALANCE_TEMP, V4L2_CID_WHITE_BALANCE_TEMPERATURE,  "White Balance Temp",            4600,  2800,   6500 );
	addSupportedSetting( CAMERA_BACKLIGHT_COMP,     V4L2_CID_BACKLIGHT_COMPENSATION,     "Backlight Comp.",                  0,     0,      1 );
	addSupportedSetting( CAMERA_POWER_FREQUENCY,    V4L2_CID_POWER_LINE_FREQUENCY,       "Powerline Hz (0=off, 1=50, 2=60)", 1,     0,      2 );
}
