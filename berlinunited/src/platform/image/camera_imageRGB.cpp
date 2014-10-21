/** @file
 **
 */

#include "camera_imageRGB.h"

#include "debug.h"

#include <arpa/inet.h>
#include <zlib.h>

#include <msg_image.pb.h>

#ifdef USE_OPENCV
#include "opencv/highgui.h"
#endif


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

CameraImageRGB::CameraImageRGB(uint16_t _imageWidth, uint16_t _imageHeight, bool allocateMemory)
		: CameraImage(_imageWidth, _imageHeight, 3, allocateMemory)
{
}


/*------------------------------------------------------------------------------------------------*/

/** Returns a new OpenCV image object in YUV format.
 **
 ** @param newImageWidth    Width of image to create
 ** @param newImageHeight   Height of image to create
 **
 ** @return new OpenCV image object in YUV format, caller is responsible of freeing resources
 */

#ifdef USE_OPENCV
cv::Mat CameraImageRGB::getImageAsYUV(uint16_t newImageWidth, uint16_t newImageHeight) const {
	// we do not support scaling up
	if (newImageWidth > imageWidth)
		newImageWidth = imageWidth;
	if (newImageHeight > imageHeight)
		newImageHeight = imageHeight;

	// create OpenCV image object
	CvSize s;
	s.width  = newImageWidth;
	s.height = newImageHeight;
	cv::Mat img(s, CV_8UC3);

	// calculate scaling ratio
	uint8_t stepX = imageWidth  / newImageWidth;
	uint8_t stepY = imageHeight / newImageHeight;

	// pointer to the image data
	uint8_t* pixelPtr = (uint8_t*) img.data;

	for (uint16_t y=0; y < imageHeight; y += stepY) {
		uint8_t* rowDataStart = pixelPtr + 3*(y/stepY * newImageWidth);
		for(uint16_t x=0; x < imageWidth; x += stepX) {
			uint8_t* imageData = rowDataStart + 3 * (x/stepX);
			getPixelAsYUV(x, y, imageData, imageData+1, imageData+2);
		}
	}

	return img;
}
#endif


/*------------------------------------------------------------------------------------------------*/

/** Returns a new OpenCV image object in RGB format.
 **
 ** @param newImageWidth    Width of image to create
 ** @param newImageHeight   Height of image to create
 **
 ** @return new OpenCV image object in RGB format, caller is responsible of freeing resources
 */

#ifdef USE_OPENCV
cv::Mat CameraImageRGB::getImageAsRGB(uint16_t newImageWidth, uint16_t newImageHeight, bool bgr) const {
	// we do not support scaling up
	if (newImageWidth > imageWidth)
		newImageWidth = imageWidth;
	if (newImageHeight > imageHeight)
		newImageHeight = imageHeight;

	cv::Mat img(cvSize(newImageWidth, newImageHeight), CV_8UC3);

	cv::Mat currentImage(cvSize(imageWidth, imageHeight), CV_8UC3);
	cvSetData(&currentImage, currentData.get(), imageWidth*3);

	if (imageWidth > newImageWidth) {
		cv::Mat img2(cvSize(imageWidth, imageHeight), CV_8UC3);
		if (bgr) {
			cvCvtColor(&currentImage, &img2, CV_RGB2BGR);
		} else {
			cvCopyImage(&currentImage, &img2);
		}
		cvResize(&img2, &img, CV_INTER_LINEAR);
	} else {
		if (bgr) {
			cvCvtColor(&currentImage, &img, CV_RGB2BGR);
		} else {
			cvCopyImage(&currentImage, &img);
		}
	}
	return img;
}
#endif
