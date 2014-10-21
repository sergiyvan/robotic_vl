#include "calibrationFile.h"
#include "debug.h"

#include "utils/utils.h"

#include <stdio.h>
#include <zlib.h>


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
**/

CalibrationFile::CalibrationFile()
	: modified(false)
	, maxBlackThreshold(40)
	, minWhiteThreshold(200)
{
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
**/

CalibrationFile::~CalibrationFile() {
	calibration.Clear();
}


/*------------------------------------------------------------------------------------------------*/

/**
**/

void CalibrationFile::setModified(bool _modified) {
	modified = _modified;
}


/*------------------------------------------------------------------------------------------------*/

/**
**/

void CalibrationFile::getData(std::string *data) {
	calibration.SerializeToString(data);
}


/*------------------------------------------------------------------------------------------------*/

/** Load lookup table from file.
 **
 ** @param _filename  Name of file containing the binary lookup table data
**/

void CalibrationFile::load(const char* _filename) {
//	INFO("Load calibration from %s", _filename);
	filename = _filename;
	std::ifstream file(_filename, std::ios::in | std::ios::binary);
	if (false == file.fail()) {
		load(file);
	} else {
		WARNING("Could not read calibration from file %s, using default camera calibration values.", _filename);
	}
}

void CalibrationFile::load(void* data, uint32_t dataLength) {
	calibration.ParseFromArray(data, dataLength);
	setModified(false);
}

void CalibrationFile::load(std::istream &in) {
	calibration.ParseFromIstream(&in);
	filename = calibration.filename();
	setModified(false);
}

void CalibrationFile::load(const de::fumanoids::message::Calibration &newCalibration) {
	calibration.CopyFrom(newCalibration);
	setModified(false);
}


/*------------------------------------------------------------------------------------------------*/

/** Save calibration to file.
 **
 ** @param _filename  Name of file
**/

bool CalibrationFile::save(const char* _filename) {
	std::string backupFileName  = std::string(_filename) + ".bak";  // backup copy
	if (fileExists(_filename)) {
		rename(_filename, backupFileName.c_str());
		sync();
	}

	std::ofstream file(_filename, std::ios::out | std::ios::trunc | std::ios::binary);
	bool success = save(file);
	std::flush(file);
	file.close();

	return success;
}

bool CalibrationFile::save(std::ostream &out) {
	INFO("Saving calibration");
	if (false == out.fail()) {
		if (calibration.SerializeToOstream(&out)) {
			setModified(false);
			INFO("Calibration saved.");
			return true;
		}

		ERROR("Error saving calibration info to stream. Some field not set?");
	} else
		ERROR("Output stream for saving calibration has failed.");

	return false;
}


/*------------------------------------------------------------------------------------------------*/

/** Get size of calibration data
 **
 ** @return size of calibration data
**/

int32_t CalibrationFile::size() {
	return calibration.ByteSize();
}


/*------------------------------------------------------------------------------------------------*/

/**
 * Returns the magnitude threshold.
 * @return the magnitude threshold or -1 on error
**/

int32_t CalibrationFile::getMagnitudeThreshold() const {
	if (calibration.has_magnitudethreshold() && calibration.magnitudethreshold() > 0)
		return calibration.magnitudethreshold();
	else
		return -1;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

uint8_t CalibrationFile::getMinWhiteThreshold() const {
	if (calibration.has_white_min() && calibration.has_white_min() > 0)
		return calibration.white_min();
	else
		return 200;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
**/

uint8_t CalibrationFile::getMaxBlackThreshold() const {
	if (calibration.has_black_max() && calibration.has_black_max() > 0)
		return calibration.black_max();
	else
		return 20;
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Store the provided camera settings.
 **
 ** @param cameraSettings  Camera settings to put into the calibration file.
 **
 ** NOTE: This function does not actually save the file!.
 */

void CalibrationFile::setCameraSettings(const de::fumanoids::message::CameraSettings &cameraSettings) {
	*calibration.mutable_camerasettings() = cameraSettings;
}
