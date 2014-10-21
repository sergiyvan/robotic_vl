/** @file
 **
 ** Class to access the image data.
 **
 ** It must be taken big care that any instance of this class is not used past
 ** its lifetime (data will become invalid as soon as the next image is captured)
 */

#ifndef IMAGEYUV422_H_
#define IMAGEYUV422_H_

#include "camera_image.h"
#include "utils/colorConverter.h"

#include "ModuleFramework/Serializer.h"


/*------------------------------------------------------------------------------------------------*/

class CameraImageYUV422 : public CameraImage {
public:
	// empty constructor needed for use as a representation
	CameraImageYUV422()
		: CameraImage(0, 0, 2, false)
	{
	}

	CameraImageYUV422(uint16_t _imageWidth, uint16_t _imageHeight, bool allocateMemory=false)
		: CameraImage(_imageWidth, _imageHeight, 2, allocateMemory)
	{
	}

	virtual ~CameraImageYUV422() {}

	virtual de::fumanoids::message::ImageFormat getImageFormat() const {
		return de::fumanoids::message::ImageFormat::YUV422_IMAGE;
	}

#ifdef USE_OPENCV
	virtual cv::Mat getImageAsYUV(double scale=1) const {
		return CameraImage::getImageAsYUV(scale);
	}
	virtual cv::Mat getImageAsYUV(uint16_t newImageWidth, uint16_t newImageHeight) const;
	virtual cv::Mat getImageAsRGB(double scale=1, bool bgr=true) const {
		return CameraImage::getImageAsRGB(scale, bgr);
	}
	virtual cv::Mat getImageAsRGB(uint16_t newImageWidth, uint16_t newImageHeight, bool bgr=true) const;
#endif


	/** Retrieve the color channel values of a pixel
	 **
	 ** @param xPos        horizontal pixel position (in subset image)
	 ** @param yPos        vertical pixel position (in subset image)
	 ** @param c1          Pointer to where to store the first color channel value
	 ** @param c2          Pointer to where to store the second color channel value
	 ** @param c3          Pointer to where to store the third color channel value
	 */
	inline void getPixelChannels(uint16_t xPos, uint16_t yPos, uint8_t *c1, uint8_t *c2, uint8_t *c3) const {
		getPixelAsYUV(xPos, yPos, c1, c2, c3);
	}


	/** Retrieve the YUV values of a pixel
	 **
	 ** @param xPos        horizontal pixel position (in subset image)
	 ** @param yPos        vertical pixel position (in subset image)
	 ** @param y           Pointer to where to store the Y value
	 ** @param u           Pointer to where to store the U value
	 ** @param v           Pointer to where to store the V value
	 ** @param averageY    True iff the y value should be averaged between two pixels
	 */
	inline void getPixelAsYUV(uint16_t xPos, uint16_t yPos, uint8_t *y, uint8_t *u, uint8_t *v, bool averageY=false) const {
		yPos += imageOffsetY;
		xPos += imageOffsetX;

		if (xPos > imageWidth - 1) xPos = imageWidth - 1;
		if (yPos > imageHeight - 1) yPos = imageHeight - 1;

		register uint32_t pos = xPos + yPos * fullImageWidth;
		register uint32_t buffer = ((uint32_t*)(currentData.get()))[ pos / 2];

		if ( (pos & 1) == 0 /* even */) {
			*y = buffer;
			*u = buffer>>8;
			*v = buffer>>24;
		} else {
			*y = buffer>>16;
			*u = buffer>>8;
			*v = buffer>>24;
		}

		pixelCountergetPixelAsXXX += 1;

		if (averageY)
			*y = ( static_cast<uint8_t>(buffer) + static_cast<uint8_t>(buffer>>16) ) / 2;
	}


	/** Retrieve brightness of a pixel (Y value in YUV space)
	 **
	 ** @param xPos      x coordinate
	 ** @param yPos      y-coordinate
	 ** @param averageY  whether to average the brightness values in YUV422
	 **
	 ** @return brightness (0..255)
	 */

	inline uint8_t getPixelBrightness(uint16_t xPos, uint16_t yPos, bool averageY=false) const {
		register uint32_t pos = (imageOffsetX + xPos) + (yPos + imageOffsetY) * fullImageWidth;
		register uint32_t buffer = ((uint32_t*)currentData.get())[ pos / 2];

		if (averageY)
			return (static_cast<uint8_t>(buffer) + static_cast<uint8_t>(buffer>>16))/2;
		else if ( (pos & 1) == 0 /* even */)
			return static_cast<uint8_t>(buffer);
		else
			return static_cast<uint8_t>(buffer>>16);
	}



	/** Retrieve the RGB values of a pixel
	 **
	 ** @param xPos        horizontal pixel position (in subset image)
	 ** @param yPos        vertical pixel position (in subset image)
	 ** @param r           Pointer to where to store the R value
	 ** @param g           Pointer to where to store the G value
	 ** @param b           Pointer to where to store the B value
	 */
	inline void getPixelAsRGB(uint16_t xPos, uint16_t yPos, uint8_t *r, uint8_t *g, uint8_t *b) const {
		uint8_t y, u, v;
		getPixelAsYUV(xPos, yPos, &y, &u, &v);
		ColorConverter::yuv2rgb(y, u, v, r, g, b);
	}

	/** Retrieve the HSV values of a pixel
	 **
	 ** @param xPos        horizontal pixel position (in subset image)
	 ** @param yPos        vertical pixel position (in subset image)
	 ** @param h           Will be set to the H value
	 ** @param s           Will be set to the S value
	 ** @param v           Will be set to the V value
	 */
	inline void getPixelAsHSV(uint16_t xPos, uint16_t yPos, uint16_t& h, uint8_t& s, uint8_t& v) const {
		uint8_t r, g, b;
		getPixelAsRGB(xPos, yPos, &r, &g, &b);
		ColorConverter::rgb2hsv(r,g,b,h,s,v);
	}

	virtual void rotate180();

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & boost::serialization::base_object<CameraImage>(*this);
	}
};

REGISTER_SERIALIZATION(CameraImageYUV422, 1)

#endif
