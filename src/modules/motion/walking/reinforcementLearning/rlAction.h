/*
 * rlAction.h
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */

#include <math.h>
#include <map>
#include <string>

#ifndef RLACTION_H_
#define RLACTION_H_




class RL_Action {
public:

	std::map<std::string, int> parameters;


	/**
	 * Empty Constructor
	 */
	RL_Action();


	/**
	 * Compares two actions and returns whether their parameters are equal.
	 *
	 * @param a1 First Action
	 * @param a Second Action
	 * @return true, if both are equal, otherwise false
	 */
	static bool actionsAreEqual(const RL_Action &a1, const RL_Action &a2);


	/**
	 * Compares the action to another action and returns whether their parameters are equal.

	 * @param a The action to compare with
	 * @return true, if both are equal, otherwise false
	 */
	bool isEqualToAction(const RL_Action &a) const;


	int getParameter(std::string name) const;

	/**
	 * Prints all the parameters of the Action.
	 */
	void print() const;
};



#endif /* RLACTION_H_ */
