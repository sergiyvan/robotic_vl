#ifndef CAMERA_SENSOR_H_
#define CAMERA_SENSOR_H_

#include "platform/system/thread.h"
#include "platform/system/events.h"
#include "platform/camera/camera.h"
#include "platform/image/camera_image.h"

#include "management/calibration/calibrationFile.h"


class CameraSensor
	: public Thread
	, public MessageCallback
{
public:
	CameraSensor();
	virtual ~CameraSensor();

	bool init();

	/// name of the thread
	virtual const char* getName() const override {
		return "CameraSensor";
	}

	/// retrieve camera image
	/// TODO: should not be used!
	inline CameraImage* getCameraImage() {
		return image;
	}

	/// get the type/name of the camera currently used
	inline std::string getCameraType() {
		return cameraType;
	}

	/// get access to the camera
	inline Camera* getCamera() {
		return cam;
	}

	// set/get camera settings
	void setSettings(const de::fumanoids::message::CameraSettings &settings);
	void getSettings(de::fumanoids::message::CameraSettings &settings);

	void saveImageToFile();

	CalibrationFile& getCalibration() {
		return calibration;
	}

	virtual bool messageCallback(
	        const std::string               &messageName,
	        const google::protobuf::Message &msg,
	        int32_t                          senderID,
	        RemoteConnectionPtr             &remote) override;

protected:
	CriticalSection cs;

	virtual void threadMain() override;
	void threadInit();
	bool initCamera();

	void handleImageSaving();
	void saveImage(std::string fileName, std::string fileExtension);

private:
	Camera         *cam;           //!< Camera object
	CameraImage    *image;         //!< current image
	std::string     cameraType;    //!< holds the type of the camera e.g. quickcam, offline etc.
	CalibrationFile calibration;   //!< the camera calibration

	RemoteConnectionPtr requestRemote;
	de::fumanoids::message::CalibrationRequest pendingCalibrationRequest;

	// we are using pointers, it does not make sense to copy this class - so prevent it
	CameraSensor(const CameraSensor &) = delete;
	CameraSensor& operator=(const CameraSensor &) = delete;
};

#endif
