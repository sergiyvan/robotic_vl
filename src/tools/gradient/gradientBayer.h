#ifndef GRADIENTBAYER_H__
#define GRADIENTBAYER_H__

#include "gradient.h"
#include "platform/image/camera_imageBayer.h"

template<>
inline void GradientCalculator<CameraImageBayer>::getGradient(
	uint16_t xPos,
	uint16_t yPos,
	int16_t *grad_x,
	int16_t *grad_y,
	uint16_t *Mag,
	uint8_t scale)
{
	xPos &= ~1;
	yPos &= ~1;
	scale &= ~1;
	if (xPos > imageWidth-scale)  xPos = imageWidth-scale;
	if (yPos > imageHeight-scale - 2) yPos = imageHeight-scale-2;

	if (scale <= 2) {
		const uint8_t* pos = (const uint8_t*)imageData.get() + yPos * imageWidth;
		uint8_t g1, g2, g3, g4;

		g1 = *(pos + xPos + 1);
		g2 = *(pos + xPos + 3);
		g3 = *(pos + imageWidth*2 + xPos + 1);
		g4 = *(pos + imageWidth*2 + xPos + 3);

		int16_t grad_gx = ((int16_t)g1-g2+g3-g4);
		int16_t grad_gy = ((int16_t)g1+g2-g3-g4);

		uint16_t mag_g = (uint16_t) (abs(grad_gx) + abs(grad_gy));
		*Mag = mag_g;
		*grad_x = grad_gx;
		*grad_y = grad_gy;

		return;
	}

	/* calculates know the gradient with this method:
	 * http://en.wikipedia.org/wiki/Edge_detection#Other_first-order_methods
	 *
	 * -----------------------
	 * |      |  1/2up |     |
	 * |------+--------+-----|
	 * | 1/2l |        |-1/2r|
	 * |------+--------+-----|
	 * |      |-1/2down|     |
	 * -----------------------
	 */

	uint16_t up_rg, up_gb, left_rg, left_gb, right_rg, right_gb, down_rg, down_gb;

	uint32_t lineDifferenceWithScale = scale * imageWidth;
	const uint8_t* pos = (const uint8_t*)imageData.get() + yPos * imageWidth + xPos;


	up_rg = *((const uint16_t *)(pos - lineDifferenceWithScale));
	up_gb = *((const uint16_t *)(pos - lineDifferenceWithScale + imageWidth));

	left_rg = *((const uint16_t *)(pos - scale));
	left_gb = *((const uint16_t *)(pos + imageWidth - scale));

	right_rg = *((const uint16_t *)(pos + scale));
	right_gb = *((const uint16_t *)(pos + imageWidth + scale));

	down_rg = *((const uint16_t *)(pos + lineDifferenceWithScale));
	down_gb = *((const uint16_t *)(pos + lineDifferenceWithScale + imageWidth));

	int16_t grad_rx = (-(int16_t) (right_rg & 255) + (int16_t) (left_rg & 255)) / 2;
	int16_t grad_ry = ((int16_t) (up_rg & 255)     - (int16_t) (down_rg & 255)) / 2;

	int16_t grad_gx = (-(int16_t) (right_gb & 255) + (int16_t) (left_gb & 255)) / 2;
	int16_t grad_gy = ((int16_t) (up_gb & 255) - (int16_t) (down_gb & 255)) / 2;

	int16_t grad_bx = (-(int16_t) ((right_gb >> 8) & 255) + (int16_t) ((left_gb >> 8) & 255)) / 2;
	int16_t grad_by = ((int16_t) ((up_gb >> 8) & 255)     - (int16_t) ((down_gb >> 8) & 255)) / 2;

	int16_t intensityDiffy = 0;//(grad_ry+grad_gy+grad_by)/3;
	int16_t intensityDiffx = 0;//(grad_rx+grad_gx+grad_bx)/3;

	// magnitude for grad_r
//		uint16_t mag_r = (uint16_t) FixedPointMath::isqrt(grad_rx * grad_rx + grad_ry * grad_ry);
	uint16_t mag_r = (uint16_t) (abs(grad_rx-intensityDiffx) + abs(grad_ry-intensityDiffy));

	// magnitude for grad_g
//		uint16_t mag_g = (uint16_t) FixedPointMath::isqrt(grad_gx * grad_gx + grad_gy * grad_gy);
	uint16_t mag_g = (uint16_t) (abs(grad_gx-intensityDiffx) + abs(grad_gy-intensityDiffy));

	// magnitude for grad_b
//		uint16_t mag_b = (uint16_t) FixedPointMath::isqrt(grad_bx * grad_bx + grad_by * grad_by);
	uint16_t mag_b = (uint16_t) (abs(grad_bx-intensityDiffx) + abs(grad_by-intensityDiffy));

//		*Mag = mag_g*4+mag_b+mag_r;
//		*grad_x = grad_gx+grad_bx+grad_rx;
//		*grad_y = grad_gy+grad_by+grad_ry;

	if (mag_g > mag_r) {
		if (mag_g > mag_b) {
			*Mag = mag_g;
			*grad_x = grad_gx;
			*grad_y = grad_gy;

		} else {
			*Mag = mag_b;
			*grad_x = grad_bx;
			*grad_y = grad_by;
		}
	} else {
		if (mag_r > mag_b) {
			*Mag = mag_r;
			*grad_x = grad_rx;
			*grad_y = grad_ry;
		} else {
			*Mag = mag_b;
			*grad_x = grad_bx;
			*grad_y = grad_by;
		}
	}
}

#endif
