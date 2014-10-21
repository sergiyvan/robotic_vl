/** @file
 **
 ** Abstract base class to access the image data.
 **
 ** This class needs to be derived for the different image formats.
 **
 ** Note on image access
 ** --------------------
 **
 ** The image data should be accessed using accessor functions. It is recommended
 ** that each subclass implements the following functions:
 **
 **    inline void getPixelAsYUV(const uint16_t &xPos, const uint16_t &yPos, uint8_t *y, uint8_t *u, uint8_t *v) const;
 **    inline void getPixelAsRGB(const uint16_t &xPos, const uint16_t &yPos, uint8_t *r, uint8_t *g, uint8_t *b) const;
 **
 ** This will require that calling code is aware of the subclass type, so it breaks
 ** the abstraction quite a bit. Unfortunately the alternative to use a virtual function
 ** would have a huge impact on performance as inlining would not be possible. So, for
 ** performance-critical functions where we want inlining to occur, code will need to
 ** be aware of the actual image format. This should be kept to as few functions as possible.
 **
 ** Non-performance critical functions (i.e. where the overhead of a function call is minimal
 ** compared to the number of times the function is called or the complexity of the code)
 ** will be virtual.
 **
 ** Performance measurements:
 **    Retrieving and adding up the YUV values for each pixel of a 640x480 image, repeated
 **    100 times on a Gumstix 600 MHz, takes the following times:
 **           20 seconds - no function call, extracting in-place
 **           20 seconds - inlined function call (current implementation)
 **           27 seconds - direct function call
 **           30 seconds - virtual function call (ideal implementation for abstraction)
 **
 **
 ** Note on image size
 ** ------------------
 **
 ** The image class supports regions of interests (ROIs):
 **
 **    +-----------------------+
 **    | captured image        |
 **    |                       |
 **    |   +---------+         |
 **    |   | image   |         |
 **    |   |         |         |
 **    |   |         |         |
 **    |   +---------+         |
 **    |                       |
 **    +-----------------------+
 **
 ** Definitions:
 **    captured image = image retrieved from camera
 **    image = subset of captured image (ROI)
 **
 ** By default, the ROI covers the whole image.
 */

#ifndef CAMERA_IMAGE_H_
#define CAMERA_IMAGE_H_

#include "ModuleFramework/Serializer.h"

#include "platform/system/timer.h"

#include "debug.h"

#include <boost/serialization/binary_object.hpp>

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_OPENCV
#include <opencv/cv.h>
#endif

#include <string>
#include <memory>


/*------------------------------------------------------------------------------------------------*/

class CameraImageRGB;

class CameraImage {
public:
	CameraImage(
		uint16_t imageWidth,
		uint16_t imageHeight,
		uint8_t  pixelSize,
		bool     allocateMemory=false);
	virtual ~CameraImage();

	/// Sets the region of interest
	virtual void setRegionOfInterest(
			uint16_t regionStartX,
			uint16_t regionStartY,
			uint16_t regionWidth,
			uint16_t regionHeight);

	/// returns the type of image
	virtual de::fumanoids::message::ImageFormat getImageFormat() const = 0;

	/// return the width of the ROI
	inline uint16_t getImageWidth() const  { return imageWidth;  }

	/// return the height of the ROI
	inline uint16_t getImageHeight() const  { return imageHeight; }

	/// return the width of the full image
	inline uint16_t getFullImageWidth() const  { return fullImageWidth;  }

	/// return the height of the full image
	inline uint16_t getFullImageHeight() const  { return fullImageHeight; }

	/// return the number of bytes in one row (full image)
	inline uint16_t getRowByteSize() const  { return fullImageWidth * pixelSize; }

	/// return the X position of the top-left corner of the ROI
	inline uint16_t getRegionOfInterestStartX() const { return imageOffsetX; }

	/// return the Y position of the top-left corner of the ROI
	inline uint16_t getRegionOfInterestStartY() const { return imageOffsetY; }

	/// get image data (points to full image!) (NOT INTENDED FOR GENERAL USE)
	inline const std::shared_ptr<void> getCurrentDataPointer() const {
		return currentData;
	}

	/// get length (in bytes) of the (full) image data (NOT INTENDED FOR GENERAL USE)
	inline int getCurrentDataLength() const {
		return currentDataLength;
	}

	/// get time in milliseconds this image was captured (or 0, if data is not available)
	inline robottime_t getTimestamp() const {
		return timestamp;
	}

	/// set image
	virtual void setImage(
		  robottime_t timestamp
		, std::shared_ptr<void> newImageData
		, int         newImageDataLength
		, int16_t     newImageWidth  = 0
		, int16_t     newImageHeight = 0);

	/// free the memory
	void freeImageData();

#ifdef USE_OPENCV
	/// retrieve an OpenCV image
	virtual cv::Mat getImageAsYUV(double scale=1) const;
	virtual cv::Mat getImageAsYUV(uint16_t newImageWidth, uint16_t newImageHeight) const = 0;
	virtual cv::Mat getImageAsRGB(double scale=1, bool bgr=true) const;
	virtual cv::Mat getImageAsRGB(uint16_t newImageWidth, uint16_t newImageHeight, bool bgr=true) const = 0;
#endif

	/// set/get image position in space
	void setImagePosition(uint16_t height, int16_t pitch, int16_t roll, int16_t headAngle);
	uint16_t getImagePositionHeight()    { return camera_height;    }
	int16_t  getImagePositionPitch()     { return camera_pitch;     }
	int16_t  getImagePositionRoll()      { return camera_roll;      }
	int16_t  getImagePositionHeadAngle() { return camera_headAngle; }

	virtual void rotate180() {
		ERROR("Image rotation not supported.");
	}

	/// set image data into a PBI structure
	virtual void getImageData(de::fumanoids::message::ImageData *pbImageData, bool compress=false) const;

	static uint32_t pixelCountergetPixelAsXXX;

protected:
	// size of the ROI
	uint16_t imageWidth;
	uint16_t imageHeight;

	// coordinates of top-left corner of ROI
	uint16_t imageOffsetX;
	uint16_t imageOffsetY;

	// size of the captured image
	uint16_t fullImageWidth;
	uint16_t fullImageHeight;

	// point of view in space
	uint16_t camera_height;
	int16_t  camera_pitch;
	int16_t  camera_roll;
	int16_t  camera_headAngle;

	std::shared_ptr<void> currentData;
	int   currentDataLength;

	uint8_t pixelSize;
	robottime_t timestamp;

	/*--- SERIALIZATION ------------------------------------------------------*/

	BOOST_SERIALIZATION_SPLIT_MEMBER()
	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive & ar, const unsigned version) const {
		ar & imageWidth;
		ar & imageHeight;
		ar & imageOffsetX;
		ar & imageOffsetY;
		ar & fullImageWidth;
		ar & fullImageHeight;
		ar & camera_height;
		ar & camera_pitch;
		ar & camera_roll;
		ar & camera_headAngle;

		ar & currentDataLength;
		ar & boost::serialization::make_binary_object(const_cast<void*>(currentData.get()), currentDataLength);

		ar & pixelSize;
		ar & timestamp;

	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version) {
		ar & imageWidth;
		ar & imageHeight;
		ar & imageOffsetX;
		ar & imageOffsetY;
		ar & fullImageWidth;
		ar & fullImageHeight;
		ar & camera_height;
		ar & camera_pitch;
		ar & camera_roll;
		ar & camera_headAngle;

		ar & currentDataLength;

		currentData = std::shared_ptr<void>(malloc(currentDataLength), [](void *ptr) { free(ptr); });
		ar & boost::serialization::make_binary_object(currentData.get(), currentDataLength);

		ar & pixelSize;
		ar & timestamp;
	}
};


REGISTER_SERIALIZATION(CameraImage, 1)

/*------------------------------------------------------------------------------------------------*/
#endif
