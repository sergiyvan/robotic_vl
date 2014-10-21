/** @file
 **
 ** Class to access the image data in Bayer RGGB format.
 **
 ** It must be taken big care that any instance of this class is not used past
 ** its lifetime (data will become invalid as soon as the next image is captured)
 */

#ifndef IMAGEBAYER_H_
#define IMAGEBAYER_H_

#include "camera_image.h"
#include "utils/colorConverter.h"

#include "ModuleFramework/Serializer.h"


/*------------------------------------------------------------------------------------------------*/

// Define INTERPOLATE_BAYER to have the camera capture an image twice the size in each
// direction in order to properly interpolate the RGB values for a pixel. If this is
// not defined, we will only capture the image in the size used in the vision, resulting
// in reduced precision (2x2 quadrants will all be the same pixel)
//#define INTERPOLATE_BAYER


/*------------------------------------------------------------------------------------------------*/

class CameraImageBayer : public CameraImage {
public:
	// empty constructor needed for use as a representation
	CameraImageBayer()
		: CameraImage(0, 0, 1, false)
	{
	}

	CameraImageBayer(uint16_t _imageWidth, uint16_t _imageHeight, bool allocateMemory = false);

	virtual ~CameraImageBayer() {}

	virtual de::fumanoids::message::ImageFormat getImageFormat() const {
		return de::fumanoids::message::ImageFormat::BAYER_IMAGE;
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
		// to reduce the access to buffer, try to read the pointer to beginning and take just required 8 bits
		pixelCountergetPixelAsXXX++;

		xPos &= ~1;
		yPos &= ~1;
		if (xPos > imageWidth-2)  xPos = imageWidth-2;
		if (yPos > imageHeight-4) yPos = imageHeight-4;

		uint8_t* pos = (uint8_t*)(currentData.get()) + yPos * imageWidth;
		uint32_t rgrg, gbgb, rgrg2;
		memcpy(&rgrg,  pos + xPos, 4);
		memcpy(&gbgb,  pos + imageWidth + xPos, 4);
		memcpy(&rgrg2, pos + imageWidth*2 + xPos, 4);

//		uint8_t r1 = rgrg;
		uint8_t r2 = rgrg >> 16;

//		uint8_t g1 = rgrg >> 8;
//		uint8_t g2 = rgrg >> 24;
//		uint8_t g3 = gbgb;
		uint8_t g4 = gbgb >> 16;

		uint8_t b1 = gbgb >> 8;
		uint8_t b2 = gbgb >> 24;

//		uint8_t r3 = rgrg2;
		uint8_t r4 = rgrg2 >> 16;

//		uint8_t g5 = rgrg2 >> 8;
//		uint8_t g6 = rgrg2 >> 24;

		*r = ((int)r2+r4)>>1;
		*g = ((int)g4);
		*b = ((int)b1+b2)>>1;
		return;

		/*
#ifdef INTERPOLATE_BAYER
		//row y
		uint16_t temp1=((uint16_t *)((uint8_t*)currentData + 2 * yPos * (2*imageWidth)))[xPos + 0];
		//row y+1
		uint16_t temp2=((uint16_t *)((uint8_t*)currentData + (2 * yPos + 1) * (2*imageWidth)))[ xPos + 0];

		//first 8 bits as R in bayer pattern
		*r = temp1 & 255;

		//second 8 bits in row y and first 8 bits in row y+1 as G values in bayer pattern
		*g = (((temp1>>8) & 255) + (temp2 & 255)) / 2;

		// second 8 bits in row y+1 as B in bayer pattern
		*b = (temp2 >>8) & 255;
#else
		if (xPos == imageWidth-1)  xPos = imageWidth-2;
		if (yPos == imageHeight-1) yPos = imageHeight-2;

		uint8_t *r1 = (uint8_t*)currentData + yPos * imageWidth + xPos;
		uint8_t *r2 = (uint8_t*)currentData + (yPos + 1) * imageWidth + xPos;

		if (xPos & 1) {
			if (yPos & 1) { // x odd, y odd
				*r = r2[1];
				*b = r1[0];
				*g = (r1[1] + r2[0]) / 2;
			} else { // x odd, y even
				*r = r1[1];
				*b = r2[0];
				*g = (r1[0] + r2[1]) / 2;
			}
		} else {
			if (yPos & 1) { // x even, y odd
				*r = r2[0];
				*b = r1[1];
				*g = (r1[0] + r2[1]) / 2;
			} else { // x even, y even
				*r = r1[0];
				*b = r2[1];
				*g = (r1[1] + r2[0]) / 2;
			}
		}
#endif

		pixelCountergetPixelAsXXX += 2;
*/
	}

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & boost::serialization::base_object<CameraImage>(*this);
	}
};

REGISTER_SERIALIZATION(CameraImageBayer, 1)

#endif
