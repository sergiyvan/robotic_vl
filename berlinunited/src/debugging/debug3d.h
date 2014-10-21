#ifndef __DEBUG3D_H__
#define __DEBUG3D_H__

#include <msg_3d.pb.h>
#include <debugging/imageDebugger.h>


/** define the elements of a 3D debug representation  */
#define DEBUG_3D(option, code) \
	{ \
		static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
		if (_debug_option == nullptr) { \
			ERROR("3D debug option %s doesn't exist", std::string(option).c_str()); \
		} else if (_debug_option->enabled) { \
			DebugStream3D debug3d; \
			code \
			debug3d.send(_debug_option); \
		} \
	}

/*------------------------------------------------------------------------------------------------*/
// colors
/*------------------------------------------------------------------------------------------------*/
/** sets the color for following draw commands */
#define SETCOLOR3D(r, g, b) \
	debug3d.setColor(r, g, b);

/** sets the color for following draw commands to red */
#define SETCOLORRED3D \
	debug3d.setColor(255, 0, 0);

/** sets the color for following draw commands to green */
#define SETCOLORGREEN3D \
	debug3d.setColor(0, 255, 0);

/** sets the color for following draw commands to blue */
#define SETCOLORBLUE3D \
	debug3d.setColor(0, 0, 255);

/** sets the color for following draw commands to black */
#define SETCOLORBLACK3D \
	debug3d.setColor(0, 0, 0);

/** sets the color for following draw commands to gray */
#define SETCOLORGRAY3D \
	debug3d.setColor(128, 128, 128);

/** sets the color for following draw commands to dark gray */
#define SETCOLORDARKGRAY3D \
	debug3d.setColor(49, 79, 79);

/** sets the color for following draw commands to white */
#define SETCOLORWHITE3D \
	debug3d.setColor(255, 255, 255);

/** sets the color for following draw commands to yellow */
#define SETCOLORYELLOW3D \
	debug3d.setColor(255, 255, 0);

/** sets the color for following draw commands to cyan */
#define SETCOLORCYAN3D \
	debug3d.setColor(0, 255, 255);

/** sets the color for following draw commands to magenta */
#define SETCOLORMAGENTA3D \
	debug3d.setColor(255, 0, 255);

/** sets the color for following draw commands to orange */
#define SETCOLORORANGE3D \
	debug3d.setColor(255, 165, 0);

#define SETCOLORINDEX3D(i) \
	debug3d.setColor(RGBColors[(i)%RGBColorsCount][0], RGBColors[(i)%RGBColorsCount][1], RGBColors[(i)%RGBColorsCount][2]); \

/** draw a circle of the given radius around the center point (x,y,z) */
#define SPHERE3D(x, y, z, radius) \
	debug3d.addCircle3D(x, y, z, radius);

/** draw a point (x,y,z) */
#define POINT3D(x, y, z) \
	debug3d.addPoint3D(x, y, z);

/** enable display of axes */
#define AXES3DON \
	debug3d.addAxes(true);

/** disable display of axes */
#define AXES3DOFF \
	debug3d.addAxes(false);

/** draw a line from (x1, y1, z1) to (x2, y2, z2) */
#define LINE3D(x1, y1, z1, x2, y2, z2) \
	debug3d.addLine3D(x1, y1, z1, x2, y2, z2);

/** draw a plane at (x, y, z) with normal (nx, ny, nz) as a circle with diameter d*/
#define PLANE3D(x, y, z, nx, ny, nz, d) \
	debug3d.addPlane3D(x, y, z, nx, ny, nz, d);

/**
 * draw a gaussian distribution given by the mean (x, y, z) and the covariance
 * matrix (cov11, cov12, voc13, cov21, cov22, cov23, cov31, cov32, cov33)
 */
#define GAUSS3D(x, y, z, cov11, cov12, cov13, cov21, cov22, cov23, cov31, cov32, cov33) \
	debug3d.addGauss3D(x, y, z, cov11, cov12, cov13, cov21, cov22, cov23, cov31, cov32, cov33);

/**
 * draw a gaussian distribution given by the mean (x, y, z) and the covariance
 * matrix (cov11, cov12, voc13, cov21, cov22, cov23, cov31, cov32, cov33)
 */
#define GAUSS3D_MAT(mu, cov) \
	{ \
		arma::mat33 U, V, S; \
		arma::colvec s; \
		arma::svd(U, s, V, cov); \
		arma::mat33 baseVecs = arma::diagmat(s) * V.t(); \
		debug3d.addGauss3D(mu(0), mu(1), mu(2), baseVecs(0, 0), baseVecs(1, 0), baseVecs(2, 0), baseVecs(0, 1), baseVecs(1, 1), baseVecs(2, 1), baseVecs(0, 2), baseVecs(1, 2) , baseVecs(2, 2)); \
	}


class DebugStream3D {
protected:
	de::fumanoids::message::Debug3D commands;

public:
	DebugStream3D()
	{
	}

	/** Clear all draw commands.
	 */
	void clear() {
		commands.Clear();
	}


	/** Send all commands.
	 */
	void send(DebuggingOption *_debug_option) {
		::Debugging::getInstance().sendDebug3D(_debug_option, commands);
	}

	/** Set the active drawing color
	 **
	 ** @param option  Debug option
	 ** @param r       Red component
	 ** @param g       Green component
	 ** @param b       Blue component
	 */
	inline void setColor(
		  uint8_t r
		, uint8_t g
		, uint8_t b)
	{
		de::fumanoids::message::Debug3DCommand::Color *color = commands.add_commands()->mutable_color();
		color->set_r(r);
		color->set_g(g);
		color->set_b(b);
	}

	/** Add drawing command for a circle for 3d plotting
	 **
	 ** @param option  Debug option
	 ** @param x       x coordinate of center
	 ** @param y       y coordinate of center
	 ** @param z       z coordinate of center
	 ** @param radius  Radius
	 ** @param filled  whether circle should be filled
	 ** @param alpha   Alpha channel value
	 */
	inline void addCircle3D(
		  float x
		, float y
		, float z
		, float radius
		, bool filled=false
		, int16_t alpha=255)
	{
		de::fumanoids::message::Debug3DCommand::Circle3D *circle = commands.add_commands()->mutable_circle3d();
		circle->set_x(x);
		circle->set_y(y);
		circle->set_z(z);
		circle->set_radius(radius);
		circle->set_filled(filled);
		circle->set_alpha(alpha);
	}

	/** Add drawing command for a circle for 3d plotting
	 **
	 ** @param option  Debug option
	 ** @param x       x coordinate of center
	 ** @param y       y coordinate of center
	 ** @param z       z coordinate of center
	 */
	inline void addPoint3D(
		  float x
		, float y
		, float z)
	{
		de::fumanoids::message::Debug3DCommand::Point3D *point = commands.add_commands()->mutable_point3d();
		point->set_x(x);
		point->set_y(y);
		point->set_z(z);
	}

	/** enable axes for 3d plotting
	 **
	 ** @param option  Debug option
	 ** @param enabled enable display of axes
	 */
	inline void addAxes(
		   bool enabled=false)
	{
		de::fumanoids::message::Debug3DCommand::Axes3D *axes = commands.add_commands()->mutable_axes3d();
		axes->set_on(enabled);
	}

	inline void addGauss3D(
		  double x
		, double y
		, double z
		, double cov11
		, double cov12
		, double cov13
		, double cov21
		, double cov22
		, double cov23
		, double cov31
		, double cov32
		, double cov33)
	{
		de::fumanoids::message::Debug3DCommand::Gauss3D* gauss = commands.add_commands()->mutable_gauss3d();

		gauss->set_x((float)x);
		gauss->set_y((float)y);
		gauss->set_z((float)z);
		gauss->set_cov11((float)cov11);
		gauss->set_cov12((float)cov12);
		gauss->set_cov13((float)cov13);
		gauss->set_cov21((float)cov21);
		gauss->set_cov22((float)cov22);
		gauss->set_cov23((float)cov23);
		gauss->set_cov31((float)cov31);
		gauss->set_cov32((float)cov32);
		gauss->set_cov33((float)cov33);
	}

	/** Add drawing command for a line
	 **
	 ** @param option
	 ** @param x1
	 ** @param y1
	 ** @param z1
	 ** @param x2
	 ** @param y2
	 ** @param z2
	 */
	inline void addLine3D(
		  float x1
		, float y1
		, float z1
		, float x2
		, float y2
		, float z2)
	{
		de::fumanoids::message::Debug3DCommand::Line3D *line = commands.add_commands()->mutable_line3d();
		line->set_x1(x1);
		line->set_y1(y1);
		line->set_z1(z1);
		line->set_x2(x2);
		line->set_y2(y2);
		line->set_z2(z2);
	}

	/** Add drawing command for a plane
	 **
	 ** @param option
	 ** @param x coordinate of center
	 ** @param y coordinate of center
	 ** @param z coordinate of center
	 ** @param nx direction of normal vector
	 ** @param ny direction of normal vector
	 ** @param nz direction of normal vector
	 ** @param d diameter diameter of the plane (will be drawn as circle
	 */
	inline void addPlane3D(
		  float x
		, float y
		, float z
		, float nx
		, float ny
		, float nz
		, float d)
	{
		de::fumanoids::message::Debug3DCommand::Plane3D *plane = commands.add_commands()->mutable_plane();
		plane->set_x(x);
		plane->set_y(y);
		plane->set_z(z);
		plane->set_nx(nx);
		plane->set_ny(ny);
		plane->set_nz(nz);
		plane->set_d(d);
	}
};

#endif
