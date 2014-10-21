#include "camera_logitech_c910.h"
#include "platform/image/image.h"
#include "platform/system/timer.h"

#include <linux/videodev2.h>


REGISTER_CAMERA("c910", CameraLogitechC910, "Logitech c910 USB webcam via v4l2");


/*------------------------------------------------------------------------------------------------*/

/** The following applies to the Logitech C910 camera with UVC interface.
 **
 ** Auto-Exposure == 0: Auto mode
 ** Auto-Exposure == 1: Manual mode
 ** Auto-Exposure == 2: Shutter priority mode
 ** Auto-Exposure == 3: Aperture priority mode, camera adjusts exposure time automatically to
 **                     handle different lighting situations (default value)
 **
 ** Auto-Exposure Priority == 1: Device may vary the framerate (change exposure absolute) in
 **                              order to improve image quality
 ** Auto-Exposure Priority == 0: Device does whatever it can to try to hold the framerate.
 **
 **
 ** Sources:
 ** - http://www.quickcamteam.net/documentation/how-to/how-to-maximize-the-frame-rate
 ** - http://www.quickcamteam.net/documentation/faq/questions-about-accessing-the-exposure-time-through-directshow
 **
 ** It may also be possible to enable bayer mode:
 **   http://www.quickcamteam.net/documentation/how-to/how-to-enable-raw-streaming-on-logitech-webcams
 **
 ** Supported values reported by Logitech C910:
 **  Control Brightness                         (code: 980900, val: 128, min: 0, max: 255)
 **  Control Contrast                           (code: 980901, val: 32, min: 0, max: 255)
 **  Control Saturation                         (code: 980902, val: 32, min: 0, max: 255)
 **  Control White Balance Temperature, Auto    (code: 98090c, val: 1, min: 0, max: 1)
 **  Control Gain                               (code: 980913, val: 64, min: 0, max: 255)
 **  Control Power Line Frequency               (code: 980918, val: 2, min: 0, max: 2)
 **   Menu items:
 **   Disabled
 **   50 Hz
 **   60 Hz
 **  Control White Balance Temperature          (code: 98091a, val: 4000, min: 2800, max: 6500)
 **  Control Sharpness                          (code: 98091b, val: 72, min: 0, max: 255)
 **  Control Backlight Compensation             (code: 98091c, val: 0, min: 0, max: 1)
 ** Control Exposure, Auto                      (code: 9a0901, val: 3, min: 0, max: 3)
 **  Menu items:
 **  Auto Mode
 **  Manual Mode
 **  Shutter Priority Mode
 **  Aperture Priority Mode
 ** Control Exposure (Absolute)                 (code: 9a0902, val: 166, min: 3, max: 2047)
 ** Control Exposure, Auto Priority             (code: 9a0903, val: 0, min: 0, max: 1)
 ** Control Pan (Absolute)                      (code: 9a0908, val: 0, min: -36000, max: 36000)
 ** Control Tilt (Absolute)                     (code: 9a0909, val: 0, min: -36000, max: 36000)
 ** Control Focus (absolute)                    (code: 9a090a, val: 68, min: 0, max: 255)
 ** Control Focus, Auto                         (code: 9a090c, val: 1, min: 0, max: 1)
 ** Control Zoom, Absolute                      (code: 9a090d, val: 1, min: 1, max: 5)
 */

CameraLogitechC910::CameraLogitechC910() {
	addSupportedSetting( CAMERA_BRIGHTNESS,         V4L2_CID_BRIGHTNESS,                 "Brightness",                     128,     0,    255 );
	addSupportedSetting( CAMERA_CONTRAST,           V4L2_CID_CONTRAST,                   "Contrast",                        32,     0,    255 );
	addSupportedSetting( CAMERA_SATURATION,         V4L2_CID_SATURATION,                 "Saturation",                      32,     0,    255 );
	addSupportedSetting( CAMERA_GAIN,               V4L2_CID_GAIN,                       "Gain",                            32,     0,    255 );
	addSupportedSetting( CAMERA_SHARPNESS,          V4L2_CID_SHARPNESS,                  "Sharpness",                      224,     0,    255 );
	addSupportedSetting( CAMERA_AUTO_EXPOSURE,      V4L2_CID_EXPOSURE_AUTO,              "Auto-Exposure (1=off)",            1,     1,      3 );
	addSupportedSetting( CAMERA_AUTO_EXPOSURE_PRIO, V4L2_CID_EXPOSURE_AUTO_PRIORITY,     "Auto-Exposure Priority (0=const)", 0,     0,      1 );
	addSupportedSetting( CAMERA_EXPOSURE,           V4L2_CID_EXPOSURE_ABSOLUTE,          "Exposure Absolute",              160,     3,   2047 );
	addSupportedSetting( CAMERA_AUTO_WHITE_BALANCE, V4L2_CID_AUTO_WHITE_BALANCE,         "Auto White Balance",               0,     0,      1 );
	addSupportedSetting( CAMERA_WHITE_BALANCE_TEMP, V4L2_CID_WHITE_BALANCE_TEMPERATURE,  "White Balance Temp",            4000,  2800,   6500 );
	addSupportedSetting( CAMERA_FOCUS_AUTO,         V4L2_CID_FOCUS_AUTO,                 "Auto-Focus (0=off)",               0,     0,      1 );
	addSupportedSetting( CAMERA_FOCUS,              V4L2_CID_FOCUS_ABSOLUTE,             "Focus",                            0,     0,    255 );
	addSupportedSetting( CAMERA_ZOOM,               V4L2_CID_ZOOM_ABSOLUTE,              "Zoom",                             1,     1,      5 );
	addSupportedSetting( CAMERA_TILT,               V4L2_CID_TILT_ABSOLUTE,              "Tilt",                             0, -36000, 36000 );
	addSupportedSetting( CAMERA_PAN,                V4L2_CID_PAN_ABSOLUTE,               "Pan",                              0, -36000, 36000 );
	addSupportedSetting( CAMERA_BACKLIGHT_COMP,     V4L2_CID_BACKLIGHT_COMPENSATION,     "Backlight Comp.",                  1,     0,      1 );
	addSupportedSetting( CAMERA_POWER_FREQUENCY,    V4L2_CID_POWER_LINE_FREQUENCY,       "Powerline Hz (0=off, 1=50, 2=60)", 1,     0,      2 );
}
