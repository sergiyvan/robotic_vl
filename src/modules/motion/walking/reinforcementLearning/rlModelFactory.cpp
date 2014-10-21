/*
 * rlmodelfactory.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */

#include "rlModelFactory.h"



RL_Model* RL_ModelFactory::createSimpleModel() {

	RL_Model* model = new RL_Model;

	model->addStateParameter("time",   0,   0,  1, 0);

	model->addStateParameter("gyroRoll", 0, -3, 3, 2);
	model->addStateParameter("gyroPitch", 0, -3, 3, 2);

	model->addStateParameter("speedX", 0, -5, 5, 2);
	model->addStateParameter("speedY", 0,  -5,  5, 2);

	model->addStateParameter("x",      0,   -1,  1, 1);

	model->addStateParameter("y",      0,   -1,  1, 1);
	model->addStateParameter("otherX",      0,   -1,  1, 1);
	model->addStateParameter("otherY",      0,   -1,  1, 1);


	model->addActionParameter("x", 0, -1, 1, 2);
	model->addActionParameter("y", 0, -1, 1, 2);


	return model;
}


