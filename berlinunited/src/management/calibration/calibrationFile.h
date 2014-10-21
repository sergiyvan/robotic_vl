/** Representation of the calibration.dat file
 **
 ** This file contains calibration information for the camera. We are separating
 ** this information from the configuration file as it presumably is identical
 ** amongst all robots (which the configuration is not).
 */

#ifndef _CALIBRATION_FILE_H_
#define _CALIBRATION_FILE_H_


#include "ModuleFramework/Serializer.h"

#include <msg_calibration.pb.h>

#include <string>
#include <fstream>


class CalibrationFile {
public:
	CalibrationFile();
	virtual ~CalibrationFile();

	void load(const char *filename);
	void load(void* data, uint32_t dataLength);
	void load(std::istream &in);
	void load(const de::fumanoids::message::Calibration &newCalibration);

	bool save(const char* filename);
	bool save(std::ostream &out);
	void empty();

	int32_t size();
	void getData(std::string *data);

	void setModified(bool modified=true);
	bool isModified() const { return modified; }
	std::string getName() const { return filename; }

	int32_t getMagnitudeThreshold() const;
	uint8_t getMaxBlackThreshold() const;
	uint8_t getMinWhiteThreshold() const;

	const de::fumanoids::message::Calibration& getProtobufCalibration() const {
		return calibration;
	}

	void setCameraSettings(const de::fumanoids::message::CameraSettings &cameraSettings);

protected:
	de::fumanoids::message::Calibration calibration;

	std::string filename;
	bool modified;

	uint8_t maxBlackThreshold;
	uint8_t minWhiteThreshold;
};

/*------------------------------------------------------------------------------------------------*/


#endif
