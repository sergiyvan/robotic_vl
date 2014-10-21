#ifndef Kinematic2013_H_
#define Kinematic2013_H_

#include "kinematic.h"

class Kinematic2013: public Kinematic {

public:
	Kinematic2013();

private:

	const int hipLength;
	const int upperLeg;
	const int kneeLength;
	const int lowerLeg;
	const int ankleLength;
	const int footHeight;


	/** Calculate the motor positions of an effector (in this case of a leg) for the given posture.
	 **
	 ** @param posture          Posture to set (x, y, z, yaw)
	 ** @param effector         Which effector to set (which leg)
	 ** @param anglesInDegree   Whether the yaw-angle in the posture is in degree
	 ** @param goal             On return contains the motor values to set
	 */
	virtual std::map<MotorID, Degree> inverseLegKinematics(const EndEffectorPose& posture, const EndEffector effector) const override;

	/**
	 * Calculates the intersecting point of two circles.
	 * Usually there are two intersecting points. The function will provide the one with greater x-value.
	 * If there is no intersection point, sx and sy will have the value -1.
	 * This function was tested. It works like charm.
	 *
	 * @param x0 x-coord of the first circle
	 * @param y0 y-coord of the first circle
	 * @param r0 radius of the first circle
	 * @param x1 x-coord of the second circle
	 * @param y1 y-coord of the second circle
	 * @param r1 radius of the second circle
	 * @param sx On return this contains the x-value of the intersecting point
	 * @param sy On return this contains the y-value of the intersecting point
	 */
	void getIntersection(const double x0, const double y0, const double r0, const double x1, const double y1, const double r1, double& sx, double& sy) const;

};

#endif

