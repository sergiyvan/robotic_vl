#ifndef IMAGEDEBUG_H_
#define IMAGEDEBUG_H_

#include "debugging.h"
#include "imageRegistration.h"

#include "utils/units.h"

#include <msg_image.pb.h>
#include <msg_drawing.pb.h>

#define DEFAULTIMAGE 1

/** define a block of draw commands for the given debug option on a given image*/
#define DRAWDEBUG(option, code) \
	{ \
		static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
		if (_debug_option == nullptr) { \
			ERROR("debug option %s doesn't exist", std::string(option).c_str()); \
		} else if (_debug_option->enabled) { \
			static __attribute__ ((unused))ImageDebugger &imageDebugger = ::Debugging::getInstance().getImageDebugger(DEFAULTIMAGE); \
			code \
		} \
	}

/*------------------------------------------------------------------------------------------------*/
// colors
/*------------------------------------------------------------------------------------------------*/
/** sets the color for following draw commands */
#define SETCOLOR(r, g, b) \
	imageDebugger.setColor(_debug_option, r, g, b);

/** sets the color for following draw commands to red */
#define SETCOLORRED \
	imageDebugger.setColor(_debug_option, 255, 0, 0);

/** sets the color for following draw commands to green */
#define SETCOLORGREEN \
	imageDebugger.setColor(_debug_option, 0, 255, 0);

/** sets the color for following draw commands to blue */
#define SETCOLORBLUE \
	imageDebugger.setColor(_debug_option, 0, 0, 255);

/** sets the color for following draw commands to black */
#define SETCOLORBLACK \
	imageDebugger.setColor(_debug_option, 0, 0, 0);

/** sets the color for following draw commands to gray */
#define SETCOLORGRAY \
	imageDebugger.setColor(_debug_option, 128, 128, 128);

/** sets the color for following draw commands to dark gray */
#define SETCOLORDARKGRAY \
	imageDebugger.setColor(_debug_option, 49, 79, 79);

/** sets the color for following draw commands to white */
#define SETCOLORWHITE \
	imageDebugger.setColor(_debug_option, 255, 255, 255);

/** sets the color for following draw commands to yellow */
#define SETCOLORYELLOW \
	imageDebugger.setColor(_debug_option, 255, 255, 0);

/** sets the color for following draw commands to cyan */
#define SETCOLORCYAN \
	imageDebugger.setColor(_debug_option, 0, 255, 255);

/** sets the color for following draw commands to magenta */
#define SETCOLORMAGENTA \
	imageDebugger.setColor(_debug_option, 255, 0, 255);

/** sets the color for following draw commands to orange */
#define SETCOLORORANGE \
	imageDebugger.setColor(_debug_option, 255, 165, 0);

/** list of rgb colors for chart coloring, without greenish colors */
#define RGBColorsCount 26
static const uint8_t RGBColors[RGBColorsCount][3] =
	{
		  {   0,   0,   0 } // black
		, { 255,   0,   0 } // red
		, {   0, 255,   0 } // green
		, {   0,   0, 255 } // blue
		, { 255, 255,   0 } // yellow
		, { 255,   0, 255 } // magenta
		, {   0, 255, 255 } // cyan
		, { 128,   0,   0 } //
		, { 128, 128,   0 } //
		, { 128,   0, 128 } //
		, {   0, 128, 128 } //
		, { 128, 128, 128 } //
		, { 153, 153, 255 } //
		, { 153,  51, 102 } //
		, { 255, 128, 128 } //
		, {   0, 102, 204 } //
		, { 204, 204, 255 } //
		, { 255, 153, 204 } //
		, {  51, 102, 255 } //
		, { 255, 153,   0 } //
		, { 255, 102,   0 } //
		, { 102, 102, 153 } //
		, { 153,  51,   0 } //
		, {  51,  51,   0 } //
		, {  51,  51, 153 } //
		, {  51,  51,  51 } //
	};

#define SETCOLORINDEX(i) \
	imageDebugger.setColor(_debug_option, RGBColors[(i)%RGBColorsCount][0], RGBColors[(i)%RGBColorsCount][1], RGBColors[(i)%RGBColorsCount][2]); \


/*------------------------------------------------------------------------------------------------*/
// draw primitives
/*------------------------------------------------------------------------------------------------*/

/** draw a circle of the given radius around the center point (x,y) */
#define CIRCLE(x, y, radius) \
	imageDebugger.addCircle(_debug_option, x, y, radius);

/** draw a filled circle of the given radius around the center point (x,y) */
#define CIRCLEFILLED(x, y, radius) \
	imageDebugger.addCircle(_debug_option, x, y, radius, true);

/** draw a filled circle of the given radius around the center point (x,y) with the given alpha */
#define CIRCLEFILLEDA(x, y, radius, alpha) \
	imageDebugger.addCircle(_debug_option, x, y, radius, true, alpha);

/** draw a rectangle with the upper left point (x,y) and the given width and height */
#define RECTANGLE(x, y, width, height) \
	imageDebugger.addRectangle(_debug_option, x, y, width, height);

/** draw a filled rectangle with the upper left point (x,y) and the given width and height */
#define RECTANGLEFILLED(x, y, width, height) \
	imageDebugger.addRectangle(_debug_option, x, y, width, height, true);

/** draw a filled rectangle with the upper left point (x,y) and the given width, height and alpha */
#define RECTANGLEFILLEDA(x, y, width, height, alpha) \
	imageDebugger.addRectangle(_debug_option, x, y, width, height, true, alpha);

/** draw a barchart which bottom left corner (x,y), height and with.
 * The value must be scaled with the height. E.g. if height == 100, then value 50
 * corresponds to 50%
 */
#define BARCHART(x, y, width, height, value) \
	imageDebugger.addBarchart(_debug_option, x, y, width, height, value);

/** draw a tetragon with the points (x1,y1), (x2,y2), (x3,y3), and (x4,y4) */
#define TETRAGON(x1, y1, x2, y2, x3, y3, x4, y4) \
	imageDebugger.addTetragon(_debug_option, x1, y1, x2, y2, x3, y3, x4, y4);

/** draw a filled tetragon with the points (x1,y1), (x2,y2), (x3,y3), and (x4,y4) */
#define TETRAGONFILLED(x1, y1, x2, y2, x3, y3, x4, y4) \
	imageDebugger.addTetragon(_debug_option, x1, y1, x2, y2, x3, y3, x4, y4, true);

/**
 * draw a gaussian distribution given by the mean (x, y) and the covariance
 * matrix (cov11, cov12, cov21, cov22)
 */
#define GAUSS2D(x, y, cov11, cov12, cov21, cov22) \
	imageDebugger.addGauss2D(_debug_option, x, y, cov11, cov12, cov21, cov22);

/** draw a line from (x1, y1) to (x2, y2) */
#define LINE(x1, y1, x2, y2) \
	imageDebugger.addLine(_debug_option, x1, y1, x2, y2);

/** draw an arrow from (x,y) pointing in the given direction with the given length */
#define ARROW(x, y, direction, length) \
	imageDebugger.addArrow(_debug_option, x, y, direction, length);

/** draw the given text, the upper left corner is defined by (x,y) */
#define TEXT(x, y, text) \
	imageDebugger.addText(_debug_option, x, y, text);


/** draw a circular arc of the given radius around the center point (x,y), starting at startAngle (in degrees) and traveling centralAngle degrees counter-clockwise */
#define CIRCULARARC(x, y, radius, startAngle, centralAngle) \
	imageDebugger.addCircularArc(_debug_option, x, y, radius, startAngle, centralAngle);

/** draw a filled circular arc of the given radius around the center point (x,y), starting at startAngle (in degrees) and traveling centralAngle degrees counter-clockwise */
#define CIRCULARARCFILLED(x, y, radius, startAngle, centralAngle) \
	imageDebugger.addCircularArc(_debug_option, x, y, radius, startAngle, centralAngle, true);


/*------------------------------------------------------------------------------------------------*/

/** The ImageDebugger holds the debug drawings, and provides an interface to
 ** easily add these drawings.
 */

class ImageDebugger : public MessageCallback {
public:
	ImageDebugger(const std::string &name, uint32_t type);
	virtual ~ImageDebugger();

	const std::string &getName() const {
		return name;
	}

	void setImageStreaming(bool enabled, Millisecond interval, std::shared_ptr<RemoteConnection> connection);
	bool requiresImageData();

	void setImage(const de::fumanoids::message::Image &pbImage);
	void streamImage(int frameNumber);

	virtual bool messageCallback(
			const std::string               &messageName,
			const google::protobuf::Message &msg,
			int32_t                          robot_id,
			RemoteConnectionPtr             &remote) override;

	/** Clear all draw commands.
	 */
	void clear() {
		drawCommands.clear();
	}

	/** Set the active drawing color
	 **
	 ** @param option  Debug option
	 ** @param r       Red component
	 ** @param g       Green component
	 ** @param b       Blue component
	 */
	inline void setColor(
		  const DebuggingOption *option
		, uint8_t r
		, uint8_t g
		, uint8_t b)
	{
		de::fumanoids::message::Drawing::Color *color = drawCommands[option->name].add_drawings()->mutable_color();
		color->set_r(r);
		color->set_g(g);
		color->set_b(b);
	}

	/** Add drawing command for a circle
	 **
	 ** @param option  Debug option
	 ** @param x       x coordinate of center
	 ** @param y       y coordinate of center
	 ** @param radius  Radius
	 ** @param filled  whether circle should be filled
	 ** @param alpha   Alpha channel value
	 */
	inline void addCircle(
		  const DebuggingOption *option
		, int16_t x
		, int16_t y
		, uint16_t radius
		, bool filled=false
		, int16_t alpha=255)
	{
		de::fumanoids::message::Drawing::Circle *circle = drawCommands[option->name].add_drawings()->mutable_circle();
		circle->set_x(x);
		circle->set_y(y);
		circle->set_radius(radius);
		circle->set_filled(filled);
		circle->set_alpha(alpha);
	}

	inline void addCircle(
		  const DebuggingOption *option
		, Centimeter x
		, Centimeter y
		, Centimeter radius
		, bool filled=false
		, int16_t alpha=255)
	{
		addCircle(option, x.value(), y.value(), radius.value(), filled, alpha);
	}

	inline void addCircle(
		  const DebuggingOption *option
		, Centimeter x
		, Centimeter y
		, int radius
		, bool filled=false
		, int16_t alpha=255)
	{
		addCircle(option, x.value(), y.value(), radius, filled, alpha);
	}

	/** Add drawing command for a rectangle
	 **
	 ** @param option  Debug option
	 ** @param x       x coordinate of top-left corner
	 ** @param y       y coordinate of top-left corner
	 ** @param width   width
	 ** @param height  height
	 ** @param filled  whether rectangle should be filled
	 ** @param alpha   alpha channel value
	 */
	inline void addRectangle(
		  const DebuggingOption *option
		, int16_t x
		, int16_t y
		, uint16_t width
		, uint16_t height
		, bool filled=false
		, int16_t alpha=255)
	{
		de::fumanoids::message::Drawing::Rectangle *rectangle = drawCommands[option->name].add_drawings()->mutable_rectangle();
		rectangle->set_x(x);
		rectangle->set_y(y);
		rectangle->set_width(width);
		rectangle->set_height(height);
		rectangle->set_filled(filled);
		rectangle->set_alpha(alpha);
	}

	inline void addRectangle(
		  const DebuggingOption *option
		, Centimeter x
		, Centimeter y
		, uint16_t width
		, uint16_t height
		, bool filled=false
		, int16_t alpha=255)
	{
		addRectangle(option, x.value(), y.value(), width, height, filled, alpha);
	}

	inline void addRectangle(
		  const DebuggingOption *option
		, Centimeter x
		, Centimeter y
		, Centimeter width
		, Centimeter height
		, bool filled=false
		, int16_t alpha=255)
	{
		addRectangle(option, x.value(), y.value(), width.value(), height.value(), filled, alpha);
	}

	/** Add drawing command for a bar chart
	 **
	 ** @param _debug_option Debug option
	 ** @param x             x coordinate of top-left corner
	 ** @param y             y coordinate of top-left corner
	 ** @param width         width
	 ** @param height        height
	 ** @param value         value to be shown
	 */
	inline void addBarchart(
		  const DebuggingOption* _debug_option
		, int16_t x
		, int16_t y
		, uint16_t width
		, uint16_t height
		, int16_t value)
	{
		setColor(_debug_option, 255, 255, 255);
		addRectangle(_debug_option, (int16_t)(x - width), (int16_t)(y + height), width, height, true);
		setColor(_debug_option, 0, 0, 255);
		addRectangle(_debug_option, (int16_t)(x - width), (int16_t)(y + value), width, value, true);
	}

	/** Add drawing command for a tetragon.
	 **
	 ** @param option  Debug option
	 ** @param x1      x-coordinate of first point
	 ** @param y1      y-coordinate of first point
	 ** @param x2      x-coordinate of second point
	 ** @param y2      y-coordinate of second point
	 ** @param x3      x-coordinate of third point
	 ** @param y3      y-coordinate of third point
	 ** @param x4      x-coordinate of fourth point
	 ** @param y4      y-coordinate of fourth point
	 ** @param filled  whether the tetragon should be filled
	 */
	inline void addTetragon(
		  const DebuggingOption *option
		, const int16_t x1
		, const int16_t y1
		, const int16_t x2
		, const int16_t y2
		, const int16_t x3
		, const int16_t y3
		, const int16_t x4
		, const int16_t y4
		, bool filled=false)
	{
		de::fumanoids::message::Drawing::Tetragon *tetragon =
				drawCommands[option->name].add_drawings()->mutable_tetragon();
		tetragon->set_x1(x1);
		tetragon->set_y1(y1);

		tetragon->set_x2(x2);
		tetragon->set_y2(y2);

		tetragon->set_x3(x3);
		tetragon->set_y3(y3);

		tetragon->set_x4(x4);
		tetragon->set_y4(y4);

		tetragon->set_filled(filled);
	}

	/** Add drawing command for a 2D gaussian
	 **
	 ** @param option  Debug option
	 ** @param x
	 ** @param y
	 ** @param cov11
	 ** @param cov12
	 ** @param cov21
	 ** @param cov22
	 */
	inline void addGauss2D(
		  const DebuggingOption *option
		, double x
		, double y
		, double cov11
		, double cov12
		, double cov21
		, double cov22)
	{
		de::fumanoids::message::Drawing::Gauss2D* gauss =
			drawCommands[option->name].add_drawings()->mutable_gauss2d();

		gauss->set_x((float)x);
		gauss->set_y((float)y);
		gauss->set_cov11((float)cov11);
		gauss->set_cov12((float)cov12);
		gauss->set_cov21((float)cov21);
		gauss->set_cov22((float)cov22);
	}

	/** Add drawing command for a line
	 **
	 ** @param option
	 ** @param x1
	 ** @param y1
	 ** @param x2
	 ** @param y2
	 */
	inline void addLine(
		  const DebuggingOption *option
		, int16_t x1
		, int16_t y1
		, int16_t x2
		, int16_t y2)
	{
		de::fumanoids::message::Drawing::Line *line = drawCommands[option->name].add_drawings()->mutable_line();
		line->set_x1(x1);
		line->set_y1(y1);
		line->set_x2(x2);
		line->set_y2(y2);
	}

	inline void addLine(
		  const DebuggingOption *option
		, Centimeter x1
		, Centimeter y1
		, Centimeter x2
		, Centimeter y2)
	{
		addLine(option, x1.value(), y1.value(), x2.value(), y2.value());
	}

	/** Add drawing command for an arrow.
	 **
	 ** @param option
	 ** @param x
	 ** @param y
	 ** @param direction
	 ** @param length
	 */
	inline void addArrow(
		  const DebuggingOption *option
		, int16_t x
		, int16_t y
		, Degree direction
		, uint16_t length)
	{
		de::fumanoids::message::Drawing::Arrow *arrow = drawCommands[option->name].add_drawings()->mutable_arrow();
		arrow->set_x(x);
		arrow->set_y(y);
		arrow->set_direction((int)(Degree(direction).value()));
		arrow->set_length(length);
	}

	/** Add drawing command for an arrow.
	 ** @deprecated
	 **
	 ** @param option
	 ** @param x
	 ** @param y
	 ** @param direction
	 ** @param length
	 */
	inline void addArrow(
		  const DebuggingOption *option
		, int16_t x
		, int16_t y
		, int16_t direction
		, uint16_t length)
	{
		addArrow(option, x, y, Degree(direction*degrees), length);
	}

	/** Add drawing command for an arrow.
	 **
	 ** @param option
	 ** @param x
	 ** @param y
	 ** @param direction
	 ** @param length
	 */
	inline void addArrow(
		  const DebuggingOption *option
		, int16_t x
		, int16_t y
		, Radian direction
		, uint16_t length)
	{
		addArrow(option, x, y, Degree(direction), length);
	}

	inline void addArrow(
		  const DebuggingOption *option
		, Centimeter x
		, Centimeter y
		, Radian direction
		, uint16_t length)
	{
		addArrow(option, x.value(), y.value(), Degree(direction), length);
	}

	inline void addArrow(
		  const DebuggingOption *option
		, Centimeter x
		, Centimeter y
		, Degree direction
		, uint16_t length)
	{
		addArrow(option, x.value(), y.value(), direction, length);
	}

	/** Add drawing command for text
	 **
	 ** @param option
	 ** @param x
	 ** @param y
	 ** @param text
	 */
	inline void addText(
		  const DebuggingOption *option
		, int16_t x
		, int16_t y
		, const std::string &text)
	{
		de::fumanoids::message::Drawing::Text *textMsg = drawCommands[option->name].add_drawings()->mutable_text();
		textMsg->set_x(x);
		textMsg->set_y(y);
		textMsg->set_text(text);
	}

	inline void addText(
		  const DebuggingOption *option
		, Centimeter x
		, Centimeter y
		, const std::string &text)
	{
		addText(option, int16_t(x.value()), int16_t(y.value()), text);
	}

	/** Add drawing command for circular arc
	 *
	 * @param option
	 * @param x
	 * @param y
	 * @param radius
	 * @param startAngle
	 * @param centralAngle
	 * @param filled
	 */
	inline void addCircularArc(const DebuggingOption *option, int16_t x, int16_t y, uint16_t radius, int16_t startAngle, int16_t centralAngle, bool filled=false) {
		de::fumanoids::message::Drawing::CircularArc *circularArc = drawCommands[option->name].add_drawings()->mutable_circular_arc();
		circularArc->set_x(x);
		circularArc->set_y(y);
		circularArc->set_radius(radius);
		circularArc->set_start_angle(startAngle);
		circularArc->set_central_angle(centralAngle);
		circularArc->set_filled(filled);
	}


protected:
	typedef std::map<std::string, de::fumanoids::message::Drawings> DrawCommandMap;
	DrawCommandMap drawCommands;

	CriticalSection cs;
	typedef struct StreamingClient {
		StreamingClient()
			: remote()
			, type(0)
			, countRemaining(0)
			, interval(Millisecond(1*seconds))
			, scaling(1.0)
			, lastTime(0)
			, compress(false)
			, rgbImage(false)
			, rawImage(false)
		{}

		RemoteConnectionPtr remote;

		uint32_t    type;
		int32_t     countRemaining;
		Millisecond interval;
		float       scaling;
		robottime_t lastTime;
		bool        compress;
		bool        rgbImage;
		bool        rawImage;
	} StreamingClient;

	std::string name;
	uint32_t type;

	bool manualStreaming;
	Millisecond manualStreamingInterval;
	Millisecond manualStreamingLastSent;
	std::shared_ptr<RemoteConnection> manualStreamingConnection;

	std::vector<StreamingClient*> clients;
	de::fumanoids::message::Message imageMessage;
	bool hasImage;

	friend class Debugging;
};

#endif
