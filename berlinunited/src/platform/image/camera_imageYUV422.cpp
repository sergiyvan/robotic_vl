#include "camera_imageYUV422.h"

#include "debug.h"

#include <arpa/inet.h>
#include <zlib.h>

#include "msg_image.pb.h"


/*------------------------------------------------------------------------------------------------*/

/** Returns a new OpenCV image object in YUV format.
 **
 ** @param newImageWidth    Width of image to create
 ** @param newImageHeight   Height of image to create
 **
 ** @return new OpenCV image object in YUV format, caller is responsible of freeing resources
 */

#ifdef USE_OPENCV
cv::Mat CameraImageYUV422::getImageAsYUV(uint16_t newImageWidth, uint16_t newImageHeight) const {
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
cv::Mat CameraImageYUV422::getImageAsRGB(uint16_t newImageWidth, uint16_t newImageHeight, bool bgr) const {
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

	// calculate scaling ratio (only natural numbers supported)
	uint8_t stepX = imageWidth  / newImageWidth;
	uint8_t stepY = imageHeight / newImageHeight;

	// pointer to the image data
	uint8_t* pixelPtr = (uint8_t*) img.data;

	for (uint16_t y=0; y < imageHeight; y += stepY) {
		uint8_t* rowDataStart = pixelPtr + 3*(y/stepY * newImageWidth);
		for(uint16_t x=0; x < imageWidth; x += stepX) {
			uint8_t* imageData = rowDataStart + 3 * (x/stepX);

			if (bgr)
				getPixelAsRGB(x, y, imageData+2, imageData+1, imageData);
			else
				getPixelAsRGB(x, y, imageData, imageData+1, imageData+2);
		}
	}

	return img;
}
#endif


/*------------------------------------------------------------------------------------------------*/

/** Rotate the image by 180°
 **
 */

void CameraImageYUV422::rotate180() {
	for (int y=0; y < imageHeight/2; y++) {
		uint8_t *topRowStart  = ((uint8_t*)currentData.get()) + 2*imageWidth*y;
		uint8_t *bottomRowEnd = ((uint8_t*)currentData.get()) + currentDataLength - 2*imageWidth*y;

		for (int x=0; x < imageWidth; x++) {
			// extract the y and color component address from current pixel (x, y)
			uint8_t *yTop = topRowStart + 2*x;
			uint8_t *cTop = yTop + 1;

			// extract the y component address from the pixel (W-x, H-y)
			uint8_t *yBottom = bottomRowEnd - 2*(x+1);

			// swap y components
			std::swap(*yTop, *yBottom);

			// swap color components
			if (x % 2 == 0) /* even pixel, i.e. we have u value */
					std::swap(*cTop, *(yBottom - 1));
			else /* odd pixel, i.e. we have v value */
					std::swap(*cTop, *(yBottom + 3));
		}
	}
}
