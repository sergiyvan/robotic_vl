#ifndef GRADIENT_H__
#define GRADIENT_H__

#include "platform/image/camera_image.h"

template<class T>
class GradientCalculator {
public:
	GradientCalculator(const T &image)
		: imageData(image.getCurrentDataPointer())
	{
		lineWidthInBytes = image.getRowByteSize();
		fullImageWidth = image.getFullImageWidth();
		imageWidth  = image.getImageWidth();
		imageHeight = image.getImageHeight();
		offsetX = image.getRegionOfInterestStartX();
		offsetY = image.getRegionOfInterestStartY();
	}

	void getGradient(
		uint16_t  xPos,
		uint16_t  yPos,
		int16_t  *grad_x,
		int16_t  *grad_y,
		uint16_t *Mag,
		uint8_t   scale);

protected:
	const std::shared_ptr<void> imageData;

	uint16_t lineWidthInBytes;
	uint16_t fullImageWidth;
	uint16_t imageWidth;
	uint16_t imageHeight;
	uint16_t offsetX;
	uint16_t offsetY;
};

#include "gradientYUV422.h"
#include "gradientRGB.h"
#include "gradientBayer.h"

#endif
