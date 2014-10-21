#ifndef BODYCONTOUR_H__
#define BODYCONTOUR_H__

#include "tools/positionImage.h"

class BodyContour {
public:
	BodyContour()
		: bodyContour(nullptr)
		, bodyContourSize(0)
	{
	}

	virtual ~BodyContour() {
		if (nullptr != bodyContour) {
			delete[] bodyContour;
			bodyContour = nullptr;
		}
	}

	/**
	 **
	 ** @param size
	 ** @param defaultValue
	 */

	inline void setBodyContourSize(const uint16_t size, const uint16_t defaultValue) {
		if (size != bodyContourSize) {
			delete[] bodyContour;
			bodyContour = nullptr;
		}

		if (nullptr == bodyContour) {
			bodyContourSize = size;
			bodyContour = new int16_t[bodyContourSize];
		}

		for (auto i=0; i < bodyContourSize; i++)
			bodyContour[i] = defaultValue;
	}

	/**
	 **
	 ** @param x
	 ** @param y
	 */

	inline void setBodyContourValue(int16_t x, int16_t y) {
		if (x < 0 || x >= bodyContourSize || bodyContour == nullptr)
			return;

		bodyContour[x] = y;
	}

	/** Checks whether a given pixel coordinate shows a part of our body
	 **
	 ** @param imageX  X-coordinate in image
	 ** @param imageY  Y-coordinate in image
	 ** @return true iff the pixel in the image at (x, y) is part of our body
	 */

	bool isBodyPixel(uint16_t imageX, uint16_t imageY) const {
		if (bodyContour == nullptr || imageX >= bodyContourSize)
			return false;

		return bodyContour[imageX] <= imageY;
	}

	// It's beeing assumed that the given coordinates are inside of fieldofview
	bool isBodyPixel(const PositionImage& _posImg) const {
		if (bodyContour == nullptr || !_posImg.isValid() || _posImg.getX() >= bodyContourSize || _posImg.getX() < 0)
			return false;

		return bodyContour[_posImg.getX()] <= _posImg.getY();
	}

	/** Retrieve the highest pixel (lowest int value) in the given column which
	 ** is part of the body.
	 **
	 ** @param   imageX   The pixel column (x) in the image.
	 ** @return the pixel highest in the picture that is body.
	 **
	 ** @note Upon passing invalid imageX values or undefined body contour,
	 **       a value of 0xFFFF may be returned.
	 */

	uint16_t getHighestBodyContourPixel(uint16_t imageX) const {
		if (bodyContour == nullptr || imageX >= bodyContourSize)
			return 0xFFFF;

		return bodyContour[imageX];
	}

protected:
	int16_t  *bodyContour;                      //!< body contour function. bodyContour[x] = lowest y coordinate, which belongs to the body
	uint16_t  bodyContourSize;                  //!< body contour size (corresponds to the image width)
};


#endif
