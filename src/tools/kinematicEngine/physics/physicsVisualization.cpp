/*
 * physicsVisualization.cpp
 *
 *  Created on: 26.05.2014
 *      Author: lutz
 */

#include "physicsVisualization.h"

PhysicsVisualization::PhysicsVisualization() {
}

PhysicsVisualization::~PhysicsVisualization() {
}

#ifdef DESKTOP

#include <drawstuff/drawstuff.h>

#ifdef dDOUBLE
#define dsDrawSphere dsDrawSphereD
#define dsDrawBox dsDrawBoxD
#define dsDrawLine dsDrawLineD
#endif

#define DRAW_JOINTS_TOO

static dsFunctions g_fn;
static PhysicsEnvironment *g_envToDraw;
static std::thread *g_drawingThread;

static void drawFunctionWrapper(int pause)
{
	PhysicsVisualization &instance = PhysicsVisualization::getInstance();
	instance.draw(pause);
}

static void start()
{
    // initial camera position
    static float xyz[3] = {-1, -1, 0.5};
    static float hpr[3] = {45, 0, 0};
    dsSetViewpoint (xyz,hpr);
}

static void drawGeom(dGeomID geomID)
{
//	printf("1%lu", (uint64_t)geomID);
    int gclass = dGeomGetClass(geomID);
//	printf("2\n");
    const dReal *pos = NULL;
    const dReal *rot = NULL;
    bool canDrawJoints = false;

    switch (gclass) {
        case dSphereClass:
			pos = dGeomGetPosition(geomID);
			rot = dGeomGetRotation(geomID);
            dsSetColorAlpha(0, 0.75, 0.5, 1);
            dsSetTexture (DS_CHECKERED);
            dsDrawSphere(pos, rot, dGeomSphereGetRadius(geomID));
            canDrawJoints = true;
            break;
        case dBoxClass:
        {
			pos = dGeomGetPosition(geomID);
			rot = dGeomGetRotation(geomID);
            dVector3 lengths;
            dsSetColorAlpha(1, 1, 0, 1);
            dsSetTexture (DS_WOOD);
            dGeomBoxGetLengths(geomID, lengths);
            dsDrawBox(pos, rot, lengths);
            canDrawJoints = true;
            break;
        }

        default:
        	break;
    }

    if (canDrawJoints) {
#ifdef DRAW_JOINTS_TOO
        dBodyID body = dGeomGetBody(geomID);
        int numJoints = dBodyGetNumJoints(body);
        for (int i = 0; i < numJoints; ++i)
        {
			dJointID joint = dBodyGetJoint(body, i);
			int jointClass = dJointGetType(joint);
			switch (jointClass)
			{
				case dJointTypeHinge:
				{
					dVector3 a11;
					dBodyID body1 = dJointGetBody(joint, 0);
					dBodyID body2 = dJointGetBody(joint, 1);

					if (body1 && body2) {
						const dReal* bodyPos1 =  dBodyGetPosition(body1);
						const dReal* bodyPos2 =  dBodyGetPosition(body2);
						dJointGetHingeAnchor(joint, a11);

						dsSetColor(1, 0, 0);
						dsDrawLine(a11, bodyPos1);
						dsDrawLine(a11, bodyPos2);

						dsSetColor(0, 1, 0);
						dsDrawLine(bodyPos1, bodyPos2);
					}
				}
			}
        }
#endif
    }
}


void PhysicsVisualization::setEnvironmentToDraw(PhysicsEnvironment *env) {
	// dummy implementation
	CriticalSectionLock csl(m_cs);

	if (nullptr != g_envToDraw && nullptr != g_drawingThread)
	{
		// quit current simulation
		dsStop();
		g_drawingThread->join();
		delete g_drawingThread;
		g_drawingThread = nullptr;
	}

	g_envToDraw = env;

	if (nullptr != g_envToDraw)
	{
		// setup drawing stuff
	    g_fn.version = DS_VERSION;
	    g_fn.start = &start;
	    g_fn.step = &drawFunctionWrapper;
	    g_fn.stop = 0;
	    g_fn.command = 0;
	    g_fn.path_to_textures = "textures/";

	    g_drawingThread = new std::thread(dsSimulationLoop, 0, (char**)NULL, 640, 480, &g_fn);
	}
}

void PhysicsVisualization::draw(int pause)
{
	if (nullptr != g_drawingThread && nullptr != g_envToDraw)
	{
		g_envToDraw->pauseSimulation();
	    // now we draw everything
		dSpaceID collisionSpace = g_envToDraw->getCollisionSpaceID();
		dSpaceID visualsSpace = g_envToDraw->getVisualsSpaceID();

	    uint ngeoms = dSpaceGetNumGeoms(collisionSpace);
	    for (uint i = 0; i < ngeoms; ++i) {
	        dGeomID g = dSpaceGetGeom(collisionSpace, i);
	        drawGeom(g);
	    }

	    ngeoms = dSpaceGetNumGeoms(visualsSpace);
		for (uint i = 0; i < ngeoms; ++i) {
			dGeomID g = dSpaceGetGeom(visualsSpace, i);
			drawGeom(g);
		}
		g_envToDraw->unPauseSimulation();
	}
}

#else

void PhysicsVisualization::setEnvironmentToDraw(PhysicsEnvironment *env) {
	// dummy implementation
}

void PhysicsVisualization::draw(int pause)
{
	// dummy implementation
}

#endif

