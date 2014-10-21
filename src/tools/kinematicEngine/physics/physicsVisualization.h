/*
 * physicsVisualization.h
 *
 *  Created on: 26.05.2014
 *      Author: lutz
 */

#ifndef PHYSICSVISUALIZATION_H_
#define PHYSICSVISUALIZATION_H_

#include "physicsEnvironment.h"
#include <utils/patterns/singleton.h>
#include <platform/system/thread.h>
#include <future>

class PhysicsVisualization : public Singleton<PhysicsVisualization> {

public:
	virtual ~PhysicsVisualization();

	void setEnvironmentToDraw(PhysicsEnvironment *env);

	void draw(int pause);

private:
	PhysicsVisualization();
	friend class Singleton<PhysicsVisualization>;

	CriticalSection m_cs;
};

#endif /* PHYSICSVISUALIZATION_H_ */
