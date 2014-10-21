#include "kinematic2013.h"

#include <math.h>

#include "utils/math/Common.h"
#include "platform/hardware/robot/robotDescription.h"


Kinematic2013::Kinematic2013()
	: hipLength(17)
	, upperLeg(100)
	, kneeLength(40)
	, lowerLeg(100)
	, ankleLength(-4)
	, footHeight(73)
{
}



/*------------------------------------------------------------------------------------------------*/

std::map<MotorID, Degree> Kinematic2013::inverseLegKinematics(const EndEffectorPose& posture, const EndEffector effector) const {
	// compute angles always in radian, since the trigonometric functions return radian values

	using namespace Math;

	// some servos are not identical in the legs, e.g. the knee servos
	// are facing a different direction in the two legs, so we may need
	// to negate the value
	int sign = (effector == LEFT_LEG) ? -1 : 1;

	// TODO check whether posture is possible or not
	// ...

	// calculate foot yaw - as the only yaw servo in the legs is in the
	// feet, this is rather trivial and also does not affect any other
	// servo position
	Degree foot_yaw = posture.yaw*degrees;

	/* Calculate roll angles - both hip and foot will have the same roll
	** angle, as the foot must be level to the ground
	**
	** First, we calculate the z position of the foot roll servo.
	**
	** Lateral view:
	**
	**            0 (0, 0, 0)
	**            |\
	**            | \
	**            |  \
	**          z |   X   <-- foot roll
	**            |   |
	**  posture.z |   -   <-- foot ground
	**                y
	*/

	// Get rid of the footHeight. We can do so because the feet always stay perpendicular to the ground.
	double z  = posture.z - footHeight;

	// Rotate about hipRoll so the leg points to the target posture.
	Degree hip_roll = Degree(atan2(posture.y, z)*radians);
	Degree foot_roll = hip_roll;

	// As we now shifted from space to plane, we cut off all unnecessary lengths.
	double leg_length = hipLength + upperLeg + kneeLength + lowerLeg + ankleLength;
	double short_leg_length = upperLeg + lowerLeg;

	// Calculate the current y and z position of the new end effector
	double sinHipRoll = sin(hip_roll);
	double ey = sinHipRoll * leg_length;
	double ez = sqrt(leg_length*leg_length - ey*ey);

	// Calculate the end effector pose without the unnecessary constant length
	double short_ey = sinHipRoll * short_leg_length;
	double short_ez = sqrt(short_leg_length* short_leg_length - short_ey*short_ey);

	// Calculate the difference
	//double dy = ey - short_ey;
	double dz = ez - short_ez;

	// Move target posture
	//double y = posture.y - dy;
	z -= dz;

	// We are now shifting into a 2 dimensional plane for calculation
	// of the pitch motors. This plane is based on the current leg
	// direction and the x-axis. As we are not interested in the foot
	// roll motor, we remove the ankle height

	// Calculate position of the KNEE_TOP motor in the plane
	double kneeTopX, kneeTopZ;
	getIntersection(posture.x, z, lowerLeg, 0, 0, upperLeg, kneeTopX, kneeTopZ);

	// Calculate the corresponding motor values for the knee pitch motors
	// Do not be confused: Parameters are defined as atan2(y, x) and not as atan2(x, y).
	Degree knee_top    = Degree(sign* atan2(kneeTopX, kneeTopZ) * radians);
	Degree knee_bottom = Degree(sign* atan2(posture.x - kneeTopX, z - kneeTopZ)*radians);

	std::map<MotorID, Degree> goal;
	if (effector == LEFT_LEG) {
		goal[MOTOR_LEFT_HIP_ROLL]    = hip_roll;
		goal[MOTOR_LEFT_KNEE_TOP]    = knee_top;
		goal[MOTOR_LEFT_KNEE_BOTTOM] = knee_bottom;
		goal[MOTOR_LEFT_FOOT_ROLL]   = foot_roll;
		goal[MOTOR_LEFT_FOOT_YAW]    = foot_yaw;
	} else {
		goal[MOTOR_RIGHT_HIP_ROLL]    = hip_roll;
		goal[MOTOR_RIGHT_KNEE_TOP]    = knee_top;
		goal[MOTOR_RIGHT_KNEE_BOTTOM] = knee_bottom;
		goal[MOTOR_RIGHT_FOOT_ROLL]   = foot_roll;
		goal[MOTOR_RIGHT_FOOT_YAW]    = foot_yaw;

	}
	return goal;
}



/*------------------------------------------------------------------------------------------------*/


void Kinematic2013::getIntersection(const double x0, const double y0, const double r0, const double x1, const double y1, const double r1, double& sx, double& sy) const {
	// dx and dy are the vertical And horizontal distances between the circle centers.
	int dx = x1 - x0;
	int dy = y1 - y0;

	// Determine the straight-Line distance between the centers.
	double d = sqrt((dy*dy) + (dx*dx));

	// Check for solvability.
	if (d > (r0 + r1)) {
		//no solution. circles do Not intersect
		sx = -1;
		sy = -1;
		return;
	}

	if (d < std::abs(r0 - r1)) {
		// no solution. one circle is contained in the other
		sx = -1;
		sy = -1;
		return;
	}

	// 'point 2' is the point where the Line through the circle
	// intersection points crosses the Line between the circle
	// centers.

	// Determine the distance from point 0 To point 2.
	double a = ((r0*r0) - (r1*r1) + (d*d)) / (2.0 * d);

	// Determine the coordinates of point 2.
	double x2 = x0 + (dx * a/d);
	double y2 = y0 + (dy * a/d);

	// Determine the distance from point 2 To either of the intersection points.
	double h = sqrt((r0*r0) - (a*a));

	// Now determine the offsets of the intersection points from
	// point 2.
	double rx = (0-dy) * (h/d);
	double ry = dx * (h/d);

	// Determine the x-values of the absolute intersection points.
	double xi1 = x2 + rx;
	double xi2 = x2 - rx;

	// Always take the point with the bigger x so the knee points forward.
	if (xi1 >= xi2) {
		sx = xi1;
		sy = y2 + ry;
	} else {
		sx = xi2;
		sy = y2 - ry;
	}
}

