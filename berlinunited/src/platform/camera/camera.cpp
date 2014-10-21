/** @file
 **
 **
 */

#include "camera.h"
#include "platform/image/image.h"

#include "utils/math/Math.h"
#include "debug.h"

#include "management/config/configRegistry.h"

#include "platform/system/timer.h"

#include <functional>
#include <algorithm>
#include <string>
#include <map>

/*------------------------------------------------------------------------------------------------*/

namespace {
	auto cfgTimestampOffset = ConfigRegistry::registerOption<Millisecond>("camera.timestamp.offset", 0*milliseconds, "Offset in milliseconds for camera image timestamp");
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

Camera::Camera()
	: image(0)
	, imageWidth(0)
	, imageHeight(0)
	, supportedSettings()
	, totalFrames(0)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

Camera::~Camera() {
	if (image)
		delete image;
	image = 0;
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

void Camera::addSupportedSetting(CAMERA_SETTING setting, unsigned long int id, const char* description, int32_t defaultValue, int32_t minValue, int32_t maxValue) {
	CameraSettingsData data;
	data.setting      = setting;
	data.id           = id;
	data.name         = description;
	data.minValue     = minValue;
	data.maxValue     = maxValue;
	data.defaultValue = defaultValue;
	data.currentValue = defaultValue;

	supportedSettings.push_back(data);
}


/*------------------------------------------------------------------------------------------------*/

/** Checks whether a certain setting is supported.
 **
 ** @return true iff setting is supported
 */

bool Camera::supports(CAMERA_SETTING setting) {
	return getSettingIndex(setting) != -1;
}


/*------------------------------------------------------------------------------------------------*/

/** Determines the index of the setting in the lookup table.
 **
 ** @param setting  Setting to look for in the table
 ** @return index in the lookup table, -1 if not found
 */

int16_t Camera::getSettingIndex(CAMERA_SETTING setting) {
	for (uint16_t i=0; i < supportedSettings.size(); i++) {
		if (supportedSettings[i].setting == setting) {
			return i;
		}
	}

	return -1;
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

int32_t Camera::getSetting(CAMERA_SETTING setting) {
	int16_t index = getSettingIndex(setting);
	if (index >= 0)
		return supportedSettings[index].currentValue;
	else
		return -1;
}

/*------------------------------------------------------------------------------------------------*/

/** Get the printable name of a camera setting.
 **
 ** @param setting  The setting to retrieve the printable name for
 ** @return the printable name of the @setting
 */

std::string Camera::getSettingName(CAMERA_SETTING setting) {
	int16_t index = getSettingIndex(setting);
	if (index >= 0)
		return supportedSettings[index].name;
	else
		return "Unknown setting";
}


/*------------------------------------------------------------------------------------------------*/

/** Configure the camera.
 **
 ** @param parameters  List of named parameters
 **
 ** @return true iff configuration succeeded
 */

bool Camera::configure(const de::fumanoids::message::CameraSettings &parameters) {
	//INFO("configure camera (%d settings)", parameters.cameraparameters_size());
	for (int i=0; i < parameters.cameraparameters_size(); i++) {
		const de::fumanoids::message::CameraParameter &p = parameters.cameraparameters(i);

		//printf("checking %s\n", p.name().c_str());
		for (uint16_t j=0; j < supportedSettings.size(); j++) {
			if (p.name().compare(supportedSettings[j].name) == 0) {
				setSetting(supportedSettings[j].setting, p.value());
//				printf("configured %s as %d\n", p.name().c_str(), p.value());
			}
		}
	}

	return true;
}


/*------------------------------------------------------------------------------------------------*/

/** Get list of available camera settings.
 **
 ** @return
 */

void Camera::getConfiguration(de::fumanoids::message::CameraSettings &settings) {
	// start fresh
	settings.mutable_cameraparameters()->Clear();

	for (uint32_t i=0; i < supportedSettings.size(); i++) {
		de::fumanoids::message::CameraParameter *parameter = settings.add_cameraparameters();
		parameter->set_id(supportedSettings[i].id);
		parameter->set_name(supportedSettings[i].name);
		parameter->set_min(supportedSettings[i].minValue);
		parameter->set_max(supportedSettings[i].maxValue);
		parameter->set_defaultvalue(supportedSettings[i].defaultValue);
		parameter->set_value(supportedSettings[i].currentValue);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

bool Camera::getValueRange(CAMERA_SETTING setting, int32_t &low, int32_t &high) {
	if (supports(setting) == false)
		return false;

	low  = supportedSettings[ getSettingIndex(setting) ].minValue;
	high = supportedSettings[ getSettingIndex(setting) ].maxValue;
	return true;
}


/*------------------------------------------------------------------------------------------------*/

/**
 */

void Camera::defaultConfiguration() {
	for (uint32_t i=0; i < supportedSettings.size(); i++)
		setSetting(supportedSettings[i].setting, supportedSettings[i].defaultValue);
}
