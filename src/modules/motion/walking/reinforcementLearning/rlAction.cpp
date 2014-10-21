/*
 * rlAction.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */

#include "rlAction.h"


using namespace std;



RL_Action::RL_Action() {
	// Empty!
}


/*------------------------------------------------------------------------------------------------*/


bool actionsAreEqual(const RL_Action &a1, const RL_Action &a2) {
	for (auto it(a1.parameters.rbegin()); it != a1.parameters.rend(); ++it) {
		if (a2.parameters.count(it->first)) {
			if (it->second != a2.parameters.at(it->first)) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}


/*------------------------------------------------------------------------------------------------*/


bool RL_Action::isEqualToAction(const RL_Action &a) const {
	for (auto it(parameters.rbegin()); it != parameters.rend(); ++it) {
		if (a.parameters.count(it->first)) {
			if (it->second != a.parameters.at(it->first)) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}


/*------------------------------------------------------------------------------------------------*/


int RL_Action::getParameter(std::string name) const {
	if (parameters.count(name)) {
		return parameters.at(name);
	} else {
		return 0;
	}
}

/*------------------------------------------------------------------------------------------------*/


void RL_Action::print() const {
	printf("Action Parameters:\n");
	for (auto it(parameters.rbegin()); it != parameters.rend(); ++it) {
		printf("A::%s: \t%d\n", it->first.c_str(), it->second);
	}
}


