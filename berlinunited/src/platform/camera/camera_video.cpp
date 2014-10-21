/*
 * camera_simulator.cpp
 *
 */

#if defined USE_OPENCV

#include "camera_video.h"

#include "platform/image/image.h"
#include "msg_image.pb.h"
#include "debug.h"
#include "utils/utils.h"
#include "services.h"

#include "management/config/config.h"
#include "management/config/configRegistry.h"

#include "platform/image/camera_imageYUV422.h"

#include <algorithm>
#include <memory>

REGISTER_CAMERA("video", CameraVideo, "Offline camera, using a video file");


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

CameraVideo::CameraVideo()
	: videoFileName("")
	, video(nullptr)
	, imageIdx(0)
	, lastImageCaptured(0)
{}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

CameraVideo::~CameraVideo() {
	closeCamera();

	if (image) {
		image->freeImageData();
		delete image;
		image = NULL;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

bool CameraVideo::openCamera(const char* deviceName, uint16_t requestedImageWidth, uint16_t requestedImageHeight, Hertz requestedFps) {
	videoFileName = deviceName;
	video = new cv::VideoCapture();
	if (false == video->open(videoFileName)) {
		ERROR("Could not open video file \"%s\".", videoFileName.c_str());
		delete video;
		video = nullptr;
		return false;
	}

	if (requestedFps != 0*hertz)
		fps = requestedFps;
	else {
		double videoFPS = video->get(CV_CAP_PROP_FPS);
		if (videoFPS > 1.)
			fps = videoFPS * hertz;
		else
			fps = 24*hertz;
	}

	INFO("Opened video file %s, playback starting with %.1f fps", videoFileName.c_str(), fps.value());

	image = new IMAGETYPE(requestedImageWidth, requestedImageHeight);
	return image != NULL;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 **
 */

void CameraVideo::closeCamera() {
	if (video != nullptr) {
		video->release();
		delete video;
		video = nullptr;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 ** Reads image from given command line argument --camera.device
 **
 */

bool CameraVideo::capture() {
	// keep the requested interval
	robottime_t currentTime   = getCurrentTime();
	robottime_t nextImageTime = lastImageCaptured + Millisecond(1./fps);
	if (nextImageTime > currentTime)
		delay(nextImageTime - currentTime);

	cv::Mat cvImageBGR;
	*video >> cvImageBGR;

	uint16_t imageHeight = cvImageBGR.rows;
	uint16_t imageWidth  = cvImageBGR.cols;

#if defined IMAGEFORMAT_YUV422
	// allocate an uint8_t array, initialize it to zero and provide a proper deleter
	std::shared_ptr<uint8_t> yuv422(new uint8_t[imageWidth*imageHeight*2](), std::default_delete<uint8_t[]>());
	if (!yuv422) {
		ERROR("Could not allocate memory for image conversion.");
		return false;
	}

	// convert image to yuv422
	for (int x=0; x < imageWidth; x += 2) {
		for (int y=0; y < imageHeight; y++) {
			uint8_t y1, y2, u1, u2, v1, v2;

			ColorConverter::rgb2yuv(cvImageBGR.at<cv::Vec3b>(y, x  )[0], cvImageBGR.at<cv::Vec3b>(y, x  )[1], cvImageBGR.at<cv::Vec3b>(y, x  )[2], &y1, &u1, &v1);
			ColorConverter::rgb2yuv(cvImageBGR.at<cv::Vec3b>(y, x+1)[0], cvImageBGR.at<cv::Vec3b>(y, x+1)[1], cvImageBGR.at<cv::Vec3b>(y, x+1)[2], &y2, &u2, &v2);

			yuv422.get()[2*y*imageWidth + 2*x + 0] = y1;
			yuv422.get()[2*y*imageWidth + 2*x + 1] = (v1 + v2)/2;
			yuv422.get()[2*y*imageWidth + 2*x + 2] = y2;
			yuv422.get()[2*y*imageWidth + 2*x + 3] = (u1 + u2)/2;
		}
	}

	image->setImage(0, std::static_pointer_cast<void>(yuv422), imageWidth*imageHeight*2, imageWidth, imageHeight);
#else
#error("Video support not yet implemented for this image type.");
#endif

	totalFrames++;
	lastImageCaptured = getCurrentTime();

	// proceed to next image
	imageIdx++;

	return true;
}


#endif // ifdef USE_OPENCV
