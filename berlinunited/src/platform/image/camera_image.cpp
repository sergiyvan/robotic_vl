/** @file
 **
 */

#include "camera_image.h"
#include "camera_imageRGB.h"

#include "debug.h"
#include "utils/utils.h"
#include "services.h"
#include "management/config/config.h"

#include <stdio.h>
#include <dirent.h>

#include <sstream>
#include <iomanip>

#include "image.h"


uint32_t CameraImage::pixelCountergetPixelAsXXX = 0;


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

CameraImage::CameraImage(uint16_t _imageWidth, uint16_t _imageHeight, uint8_t pixelSize, bool allocateMemory)
	: imageWidth(_imageWidth)
	, imageHeight(_imageHeight)
	, imageOffsetX(0)
	, imageOffsetY(0)
	, fullImageWidth(_imageWidth)
	, fullImageHeight(_imageHeight)
	, camera_height(0)
	, camera_pitch(0)
	, camera_roll(0)
	, camera_headAngle(0)
	, currentData(0)
	, currentDataLength(0)
	, pixelSize(pixelSize)
	, timestamp(0)
{
	if (allocateMemory) {
		timestamp = getCurrentTime();
		currentDataLength = imageWidth * imageHeight * pixelSize;
		currentData.reset(calloc(imageWidth * imageHeight, pixelSize), [](void *ptr) { free(ptr); });
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

CameraImage::~CameraImage() {
}


/*------------------------------------------------------------------------------------------------*/

/** Sets the subset of the image we are interested in
 **
 ** @param offsetX        Horizontal offset into the full image data (in pixel)
 ** @param offsetY        Vertical offset into the full image data (in pixel)
 ** @param subsetWidth    Width (in pixel) of the subset image (divisible by 4)
 ** @param subsetHeight   Height (in pixel) of the subset image
 */

void CameraImage::setRegionOfInterest(
		  uint16_t offsetX
		, uint16_t offsetY
		, uint16_t subsetWidth
		, uint16_t subsetHeight
) {
	if (offsetX < fullImageWidth)
		imageOffsetX = offsetX;
	else {
		WARNING("Image-ROI offset (X) exceeds image width, ignoring");
		imageOffsetX = 0;
	}

	if (offsetY < fullImageHeight)
		imageOffsetY = offsetY;
	else {
		WARNING("Image-ROI offset (Y) exceeds image width, ignoring");
		imageOffsetY = 0;
	}

	if (offsetX + subsetWidth <= fullImageWidth)
		imageWidth = subsetWidth;
	else {
		WARNING("Image-ROI width too large, limiting to end of image");
		imageWidth = fullImageWidth - offsetX;
	}

	if (offsetY + subsetHeight <= fullImageHeight)
		imageHeight = subsetHeight;
	else {
		WARNING("Image-ROI height too large, limiting to end of image");
		imageHeight = fullImageHeight - offsetY;
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void CameraImage::setImage(robottime_t timestamp, std::shared_ptr<void> newImageData, int newImageDataLength, int16_t newImageWidth, int16_t newImageHeight) {
	// remember values
	currentData = newImageData;
	currentDataLength = newImageDataLength;
	this->timestamp = timestamp;

	// take the gyro roll and pitch, offsets to the camera are used in the camera model
	// robottime_t time = getCurrentTime();
	// int16_t headAngle = getHeadAngles().getRoll();
	// float pitch = getGyroData().getPitch();
	// float roll  = getGyroData.getRoll();

	// FIXME
	// update the head angles right after the frame has been captured
	// to keep image and angles somehow synchronized
	// setImagePosition( ROBOT_EYE_HEIGHT,  (int16_t) round(pitch), (int16_t) round(roll), headAngle );
	// TheCameraModel::getInstance().setAngles( pitch, roll, 0, headAngle );
	// TheCameraModel::getInstance().setTimeDiff(pitchAndRoll.timediff);

	if (newImageWidth > 0) {
		fullImageWidth  = newImageWidth;
		fullImageHeight = newImageHeight;

		// reset ROI
		imageOffsetX = imageOffsetY = 0;
		imageWidth   = fullImageWidth;
		imageHeight  = fullImageHeight;
//		setImageAsSubset(imageOffsetX, imageOffsetY, imageWidth, imageHeight);
	}
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void CameraImage::setImagePosition(uint16_t height, int16_t pitch, int16_t roll, int16_t headAngle) {
	camera_height    = height;
	camera_pitch     = pitch;
	camera_roll      = roll;
	camera_headAngle = headAngle;
}


/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

void CameraImage::freeImageData() {
	if (currentData)
		currentData.reset();

	currentDataLength = 0;
	imageWidth        = 0;
	imageHeight       = 0;
}


/*------------------------------------------------------------------------------------------------*/

/** Returns a new OpenCV image object in YUV format.
 **
 ** @param scale      Scaling factor. Must be 1/1, 1/2, 1/3, ...
 **
 ** @return new OpenCV image object in YUV format, caller is responsible of freeing resources
 */

#ifdef USE_OPENCV
cv::Mat CameraImage::getImageAsYUV(double scale) const {
	if (scale > 1)
		return cv::Mat();

	return getImageAsYUV((uint16_t)(imageWidth * scale), (uint16_t)(imageHeight * scale));
}
#endif


/*------------------------------------------------------------------------------------------------*/

/** Returns a new OpenCV image object in RGB format.
 **
 ** @param scale     Scaling factor. Must be 1/1, 1/2, 1/3, ...
 **
 ** @return new OpenCV image object in RGB format, caller is responsible of freeing resources
 */

#ifdef USE_OPENCV
cv::Mat CameraImage::getImageAsRGB(double scale, bool bgr) const {
	if (scale > 1)
		return cv::Mat();

	return getImageAsRGB((uint16_t)(imageWidth * scale), (uint16_t)(imageHeight * scale), bgr);
}
#endif


/*------------------------------------------------------------------------------------------------*/

/**
 ** Adds image data to a PBI-object
 **
 ** @param pbImageData    PBI-object to set data for
 */

void CameraImage::getImageData(
		de::fumanoids::message::ImageData *pbImageData,
		bool compress) const
{
	pbImageData->set_data(currentData.get(), currentDataLength);
	pbImageData->set_compressed(false);
	pbImageData->set_width(imageWidth);
	pbImageData->set_height(imageHeight);
	pbImageData->set_format((de::fumanoids::message::ImageFormat)getImageFormat());

/*
	void *compressedData = 0;
	uint32_t dataSize = data.size();

	if (compress) {
		// printf("compressing\n");
		uLongf uncompressedSize = data.size();
		uLongf compressedSize = compressBound(uncompressedSize);

		// reserve memory for compressed image
		compressedData = malloc(compressedSize);

		if (compressedData != 0) {
			int res = ::compress2((Bytef*) compressedData, &compressedSize,
			                      (Bytef*) data.c_str(), uncompressedSize,
			                      Z_BEST_COMPRESSION);

			if (res == Z_OK && compressedSize < uncompressedSize) {
				INFO("Image compression finished (compressed to %ld (%d%%) bytes)",
				      compressedSize, compressedSize * 100 / uncompressedSize);
				dataSize = compressedSize;
			} else {
				printf("compressed size >= uncompressed size\n");
				free(compressedData);
				compressedData = 0;
			}
		}
		// else printf("no memory\n");
	}
*/
}
