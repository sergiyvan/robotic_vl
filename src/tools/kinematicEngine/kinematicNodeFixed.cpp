/*
 * kinematicNodeFixed.cpp
 *
 *  Created on: 23.09.2014
 *      Author: lutz
 */

#include <tools/kinematicEngine/kinematicNodeFixed.h>

KinematicNodeFixed::KinematicNodeFixed()
	: KinematicNodeDummy()
	, m_fixedJoint(0)
{
}

KinematicNodeFixed::KinematicNodeFixed(
		MotorID id,
		KinematicNode *parent,
		std::string name,
		Millimeter translationX,
		Millimeter translationY,
		Millimeter translationZ,
		Degree alphaX,
		Degree alphaY,
		Degree alphaZ)
	: KinematicNodeDummy(id, parent, name, translationX, translationY, translationZ, alphaX, alphaY, alphaZ)
	, m_fixedJoint(0)
{
}

KinematicNodeFixed::~KinematicNodeFixed()
{
}


dBodyID KinematicNodeFixed::attachToODE(PhysicsEnvironment *environment, dSpaceID visualSpaceID, dSpaceID collisionSpaceID, arma::mat44 coordinateFrame)
{
	KinematicNodeDummy::attachToODE(environment, visualSpaceID, collisionSpaceID, coordinateFrame);

	dBodyID body = getODEBody();
	dBodySetKinematic(body);

	return body;
}

