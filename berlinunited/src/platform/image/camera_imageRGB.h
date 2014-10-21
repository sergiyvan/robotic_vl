/** @file
 **
 ** Class to access the image data in RGB format.
 **
 ** It must be taken big care that any instance of this class is not used past
 ** its lifetime (data will become invalid as soon as the next image is captured)
 */

#ifndef IMAGERGB_H_
#define IMAGERGB_H_

#include "camera_image.h"
#include "utils/colorConverter.h"

#include "ModuleFramework/Serializer.h"


/*------------------------------------------------------------------------------------------------*/

class CameraImageRGB : public CameraImage {
public:
	// empty constructor needed for use as a representation
	CameraImageRGB()
		: CameraImage(0, 0, 3, false)
	{
	}

	CameraImageRGB(uint16_t _imageWidth, uint16_t _imageHeight, bool allocateMemory = false);

	virtual ~CameraImageRGB() {}

	virtual de::fumanoids::message::ImageFormat getImageFormat() const {
		return de::fumanoids::message::ImageFormat::RGB_IMAGE;
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

	/** Retrieve the YUV values of a pixel
	 **
	 ** @param xPos        horizontal pixel position (in subset image)
	 ** @param yPos        vertical pixel position (in subset image)
	 ** @param y           Pointer to where to store the Y value
	 ** @param u           Pointer to where to store the U value
	 ** @param v           Pointer to where to store the V value
	 ** @param averageY    (unused)
	 */
	inline void getPixelAsYUV(uint16_t xPos, uint16_t yPos, uint8_t *y, uint8_t *u, uint8_t *v, bool averageY=false) const {
		uint8_t r, g, b;
		getPixelAsRGB(xPos, yPos, &r, &g, &b);
		ColorConverter::rgb2yuv(r,g,b,y,u,v);
	}


	/** Retrieve brightness of a pixel (Y value in YUV space)
	 **
	 ** @param xPos      x coordinate
	 ** @param yPos      y-coordinate
	 ** @param averageY  (unused)
	 **
	 ** @return brightness (0..255)
	 */

	inline uint8_t getPixelBrightness(uint16_t xPos, uint16_t yPos, bool averageY=false) const {
		uint8_t r, g, b;
		getPixelAsRGB(xPos, yPos, &r, &g, &b);

		register float r_fixed = r;
		register float g_fixed = g;
		register float b_fixed = b;
		return (int) (r_fixed * 0.299f + g_fixed * 0.587f + b_fixed * 0.114f);
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


	/** Retrieve the color channel values of a pixel
	 **
	 ** @param xPos        horizontal pixel position (in subset image)
	 ** @param yPos        vertical pixel position (in subset image)
	 ** @param c1          Pointer to where to store the first color channel value
	 ** @param c2          Pointer to where to store the second color channel value
	 ** @param c3          Pointer to where to store the third color channel value
	 */
	inline void getPixelChannels(uint16_t xPos, uint16_t yPos, uint8_t *c1, uint8_t *c2, uint8_t *c3) const {
		getPixelAsRGB(xPos, yPos, c1, c2, c3);
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
		uint8_t* pos = (uint8_t*)(currentData.get()) + 3 * (yPos * imageWidth + xPos);

		*r = pos[0];
		*g = pos[1];
		*b = pos[2];
	}

	inline void setPixel(uint16_t x, uint16_t y, uint16_t r, uint8_t g, uint8_t b) {
		uint8_t *pixelStart = (uint8_t*)(currentData.get()) + 3*((y + imageOffsetY)*fullImageWidth + x + imageOffsetX);

		*(pixelStart + 0) = r;
		*(pixelStart + 1) = g;
		*(pixelStart + 2) = b;
	}

protected:
protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & boost::serialization::base_object<CameraImage>(*this);
	}
};

REGISTER_SERIALIZATION(CameraImageRGB, 1)


#endif
