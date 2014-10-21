#ifndef CAMERA_VIDEO_H_
#define CAMERA_VIDEO_H_

#ifdef USE_OPENCV

#include "camera.h"

#include <string>
#include <opencv2/opencv.hpp>

class CameraVideo : public Camera {
public:
	CameraVideo();
	virtual ~CameraVideo();

	virtual std::string getCameraName() {
		return "Offline/Video";
	}

	virtual bool openCamera(const char* deviceName, uint16_t requestedImageWidth, uint16_t requestedImageHeight, Hertz requestedFps=0*hertz) override;
	virtual void closeCamera();
	virtual bool isOpen() { return video != nullptr; }

	/// capture an image
	virtual bool capture();

	/// set a camera setting
	virtual void setSetting(CAMERA_SETTING setting, int32_t value) {}

	inline std::string getCurrentVideoName() {
		return videoFileName;
	}

	/** sets the position of the camera above the ground (height, in cm) */
	void setHeight(int32_t value) {
		robotEyeHeight = value;
	}

protected:
	/// name of currently loaded image file
	std::string videoFileName;
	cv::VideoCapture *video;

	bool readImageFromPBI(std::string cameraImageFile);
	bool readImage(std::string cameraImageFile);

	int imageIdx; // index of current image if a list of images is used
	robottime_t lastImageCaptured;

	int32_t robotEyeHeight;

	Hertz fps;
};

#endif /* USE_OPENCV */
#endif /* CAMERA_SIMULATOR_H_ */
