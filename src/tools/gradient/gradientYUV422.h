#ifndef GRADIENTYUV422_H__
#define GRADIENTYUV422_H__

#include "gradient.h"

template<>
inline void GradientCalculator<CameraImageYUV422>::getGradient(
	uint16_t xPos,
	uint16_t yPos,
	int16_t *grad_x,
	int16_t *grad_y,
	uint16_t *Mag,
	uint8_t scale)
{
	xPos += offsetX;
	yPos += offsetY;

	if (xPos<scale) xPos=scale;
	if (yPos<scale) yPos=scale;
	if (xPos>imageWidth-scale) xPos=imageWidth-scale;
	if (yPos>imageHeight-scale) yPos=imageHeight-scale;

	register uint32_t pos1 = xPos + yPos * fullImageWidth;

	/* calculates now the gradient with this method:
	 * http://en.wikipedia.org/wiki/Edge_detection#Other_first-order_methods
	 *
	 * -----------------------
	 * |      | 1/2up  |     |
	 * |------+--------+-----|
	 * | 1/2l |        |-1/2r|
	 * |------+--------+-----|
	 * |      |-1/2down|     |
	 * -----------------------
	 */

	register uint32_t pos = pos1 - scale*fullImageWidth;
	register uint32_t up = static_cast<const uint32_t*>(imageData.get())[pos / 2];

	pos = pos1 + scale*fullImageWidth;
	register uint32_t down = static_cast<const uint32_t*>(imageData.get())[pos / 2];

	pos = pos1 - scale;
	register uint32_t left = static_cast<const uint32_t*>(imageData.get())[pos / 2];

	pos = pos1 + scale;
	register uint32_t right = static_cast<const uint32_t*>(imageData.get())[pos / 2];


	int16_t grad_yx, grad_yy;

	if ((pos1 & 1) == 0 /* even */ ) {
		grad_yx = (-((right >> 16) & 255) + ((left >> 16) & 255)) / 2;
		grad_yy = (((up >> 16) & 255) - ((down >> 16) & 255)) / 2;

	} else {
		grad_yx = (-(right & 255) + (left & 255)) / 2;
		grad_yy = ((up & 255) - (down & 255)) / 2;
	}
	int16_t grad_ux = (-((right >> 8) & 255) + ((left >> 8) & 255)) / 2;
	int16_t grad_uy = (((up >> 8) & 255) - ((down >> 8) & 255)) / 2;

	int16_t grad_vx = (-((right >> 24) & 255) + ((left >> 24) & 255)) / 2;
	int16_t grad_vy = (((up >> 24) & 255) - ((down >> 24) & 255)) / 2;

	// magnitude for grad_y
	uint16_t mag_y = static_cast<uint16_t> (abs(grad_yx) + abs(grad_yy));

	// magnitude for grad_v
	uint16_t mag_v = static_cast<uint16_t> (abs(grad_vx) + abs(grad_vy));

	// magnitude for grad_u
	uint16_t mag_u = static_cast<uint16_t> (abs(grad_ux) + abs(grad_uy));

//		*Mag = mag_y + mag_v + mag_u;

	if (mag_y > mag_v) {
		if (mag_y > mag_u) {
			*Mag = mag_y;
			*grad_x = grad_yx;
			*grad_y = grad_yy;

		} else {
			*Mag = mag_u;
			*grad_x = grad_ux;
			*grad_y = grad_uy;
		}
	} else {
		if (mag_v > mag_u) {
			*Mag = mag_v;
			*grad_x = grad_vx;
			*grad_y = grad_vy;
		} else {
			*Mag = mag_u;
			*grad_x = grad_ux;
			*grad_y = grad_uy;
		}
	}
}

#endif
