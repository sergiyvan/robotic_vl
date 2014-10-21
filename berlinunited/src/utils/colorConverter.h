#ifndef COLOR_CONVERTER_H__
#define COLOR_CONVERTER_H__

#include <armadillo>
#include <inttypes.h>


/*------------------------------------------------------------------------------------------------*/

class ColorConverter {
public:
	/// convert a yuv value to an rgb one
	static inline void yuv2rgb(
				const uint8_t &y, const uint8_t &u, const uint8_t &v,
				uint8_t *r, uint8_t* g, uint8_t *b
			)
	{
		int dr = 1000 + (351*v/256) - 175;
		int dg = 1000 - (179*v/256) + 90 - (86*u/256) + 43;
		int db = 1000 + (443*u/256) - 222;

		int r1 = y + dr;
		int g1 = y + dg;
		int b1 = y + db;

		if (b1<1000) b1 = 1000;
		if (b1>1255) b1 = 1255;
		if (g1<1000) g1 = 1000;
		if (g1>1255) g1 = 1255;
		if (r1<1000) r1 = 1000;
		if (r1>1255) r1 = 1255;

		*b = b1-1000;
		*g = g1-1000;
		*r = r1-1000;
	}

	static inline arma::colvec yuv2rgb(const arma::colvec &yuv) {
		arma::colvec ret;
		ret.zeros(yuv.size(), 1);

		uint8_t r, g, b;
		yuv2rgb(yuv(0), yuv(1), yuv(2), &r, &g, &b);

		ret << r << g << b << arma::endr;
		return ret;
	}

	static inline void rgb2yuv(
			const uint8_t &r, const uint8_t &g, const uint8_t &b,
			uint8_t *y, uint8_t *u, uint8_t *v)
	{

		register float r_fixed = r;
		register float g_fixed = g;
		register float b_fixed = b;

		// Old conversion: is actually rgb to YCbCr!
//		*y = toInt(toFixed( 16) + fmul(r_fixed, toFixed(0.257)) + fmul(g_fixed, toFixed(0.504)) + fmul(b_fixed, toFixed(0.098)));
//		*u = toInt(toFixed(128) - fmul(r_fixed, toFixed(0.148)) - fmul(g_fixed, toFixed(0.291)) + fmul(b_fixed, toFixed(0.439)));
//		*v = toInt(toFixed(128) + fmul(r_fixed, toFixed(0.439)) - fmul(g_fixed, toFixed(0.368)) - fmul(b_fixed, toFixed(0.071)));

		/*
		 * Current conversion rgb to yuv
		 * @see http://softpixel.com/~cwright/programming/colorspace/yuv/
		 * Y = R *  .299 + G *  .587 + B *  .114
		 * U = R * -.169 + G * -.331 + B *  .500 + 128
		 * V = R *  .500 + G * -.418 + B * -.081 + 128
		 *
		 * seems to be inverse to our current yuv2rgb()
		 */

		*y = static_cast<int> (         r_fixed * 0.299f + g_fixed * 0.587f + b_fixed * 0.114f);
		*u = static_cast<int> (128.0f - r_fixed * 0.169f - g_fixed * 0.331f + b_fixed * 0.500f);
		*v = static_cast<int> (128.0f + r_fixed * 0.500f - g_fixed * 0.418f - b_fixed * 0.081f);
	}

	/**
	 * Convert RGB to HSV (Hue, Saturation, Value)
	 * @param r - red value
	 * @param g - green value
	 * @param b - blue value
	 * @param h - is set to the hue [0,360)
	 * @param s - is set to the saturation [0,100]
	 * @param v - is set to the value [0,100]
	 */
	static inline void rgb2hsv(
			const uint8_t &r, const uint8_t &g, const uint8_t &b,
			uint16_t &h, uint8_t &s, uint8_t &v)
	{

		float red   = r / 255.0f;
		float green = g / 255.0f;
		float blue  = b / 255.0f;
		float max = std::max(std::max(red,green),blue);
		float min = std::min(std::min(red,green),blue);
		float delta = max-min;
		float h_fixed = 0;

		if (delta == 0) h_fixed = 0;
		else if (red   == max)  h_fixed = ((green-blue) / delta) * 60.0f;
		else if (green == max)  h_fixed = (2.0f + (blue-red) / delta) * 60.0f;
		else if (blue  == max)  h_fixed = (4.0f + (red-green) / delta) * 60.0f;

		int16_t h_int = static_cast<int16_t> (h_fixed);

		if (h_int < 0) h = h_int + 360;
		else h = h_int;

		float s_fixed = (max == 0)? 0 : delta / max;

		s = static_cast<int> (s_fixed * 100.0f);
		v = static_cast<int> (max * 100.0f);
	}

	// TODO: merge with above function, but beware that this one works with float
	static inline arma::colvec rgb2hsb(arma::colvec const &rgb) {
		float red   = rgb(0) / 255.0f;
		float green = rgb(1) / 255.0f;
		float blue  = rgb(2) / 255.0f;
		float max = std::max(std::max(red, green), blue);
		float min = std::min(std::min(red, green), blue);
		float delta = max-min;
		float h_fixed = 0;

		if (delta == 0) h_fixed = 0;
		else if (red   == max)  h_fixed = ((green-blue) / delta) * 60.0f;
		else if (green == max)  h_fixed = (2.0f + (blue-red) / delta) * 60.0f;
		else if (blue  == max)  h_fixed = (4.0f + (red-green) / delta) * 60.0f;

		int16_t h_int = static_cast<int16_t> (h_fixed);

		arma::colvec hsb;
		hsb.zeros(rgb.size(), 1);


		if (h_int < 0) {
			hsb(0) = h_int + 360;
		} else {
			hsb(0) = h_int;
		}

		float s_fixed = (max == 0)? 0 : delta / max;

		hsb(1) = (s_fixed * 100.0f);
		hsb(2) = (max * 100.0f);

		return hsb;
	}
};


#endif
