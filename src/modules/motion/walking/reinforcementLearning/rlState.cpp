/*
 * rlState.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */

#include "rlState.h"
#include "rlModel.h"


using namespace std;


RL_State::RL_State() {
	bfs_visited = false;
}


/*------------------------------------------------------------------------------------------------*/


int RL_State::getParameter(string name) const {
	if (parameters.count(name)) {
		return parameters.at(name);
	} else {
		return 0;
	}
}


/*------------------------------------------------------------------------------------------------*/


void RL_State::setParameter(string name, int value, RL_Model* model) {
	if (parameters.count(name)) {
		parameters[name] = value;
		correctValuesByModel(model);
	}
}



/*------------------------------------------------------------------------------------------------*/


RL_State RL_State::getSuccessorForAction(RL_State s, RL_Action a, RL_Model* model) {
	RL_State s_succ(s);
	for (auto it(a.parameters.rbegin()); it != a.parameters.rend(); ++it) {
		s_succ.parameters[it->first] += it->second;
	}
	s_succ.correctValuesByModel(model);
	return s_succ;
}


/*------------------------------------------------------------------------------------------------*/


void RL_State::updateByAction(RL_Action a, RL_Model* model) {
	for (auto it(a.parameters.rbegin()); it != a.parameters.rend(); ++it) {
//		if (parameters.find(it->first) != parameters.end()) {
		if (parameters.count(it->first)) {
			int newValue = parameters.at(it->first) + it->second;
			parameters[it->first] = newValue;
		}
	}
	correctValuesByModel(model);
}


/*------------------------------------------------------------------------------------------------*/


bool RL_State::statesAreEqual(RL_State s1, RL_State s2) {
	for (auto it(s1.parameters.rbegin()); it != s1.parameters.rend(); ++it) {
		if (s2.parameters.count(it->first)) {
			if (it->second != s2.parameters.at(it->first)) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}


/*------------------------------------------------------------------------------------------------*/


bool RL_State::isEqualToState(RL_State s) const {
	for (auto it(parameters.rbegin()); it != parameters.rend(); ++it) {
		if (s.parameters.count(it->first)) {
			if (it->second != s.parameters.at(it->first)) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}


/*------------------------------------------------------------------------------------------------*/


void RL_State::correctValuesByModel(RL_Model* model) {
	for (auto it(parameters.begin()); it != parameters.end(); ++it) {
		// The state is not allowed to be on the border. That is why minValue and maxValue
		if (it->second < model->stateSpace[it->first].minValue) {it->second = model->stateSpace[it->first].minValue;}
		if (it->second > model->stateSpace[it->first].maxValue) {it->second = model->stateSpace[it->first].maxValue;}
	}
}


/*------------------------------------------------------------------------------------------------*/


void RL_State::print() const {
	printf("State Parameters:\n");
	for (auto it(parameters.rbegin()); it != parameters.rend(); ++it) {
		printf("S::%s: \t%d\n", it->first.c_str(), it->second);
	}
}


