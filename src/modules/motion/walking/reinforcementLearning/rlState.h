/*
 * rlState.h
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */


#include "rlAction.h"
#include <string>
#include <map>



#ifndef RLSTATE_H_
#define RLSTATE_H_



// Assert the existence of class RL_Model
class RL_Model;


class RL_State {
public:

	bool bfs_visited;
	std::map<std::string, int> parameters;

	/**
	 * In the constructor all parameters are initially set to be 0.
	 */
	RL_State();


	/**
	 * This function returns the value of a parameter, given by it's key,
	 * but only if a parameter with this key actually exists.
	 *
	 * @param name Key of the parameter
	 * @return Value of the parameter if it exists, otherwise 0
	 */
	int getParameter(std::string name) const;


	/**
	 * Sets the the Value of a Parameter given by it's key, but only if
	 * it exists.
	 *
	 * @param name Key of the Parameter
	 * @param value New Value of the Parameter
	 */
	void setParameter(std::string name, int value, RL_Model* model);


	/**
	 * Returns the successor state for for a given state and a given action,
	 * without changing the state's parameters.
	 * It calculates only the deterministic successor state's parameters, effected by the action.
	 *
	 * @param s State
	 * @param a Action
	 * @return Successor State
	 */
	static RL_State getSuccessorForAction(RL_State s, RL_Action a, RL_Model* model);


	/**
	 * Updates the state's parameters accordingly to a given action.
	 *
	 * @param a Action
	 */
	void updateByAction(RL_Action a, RL_Model* model);


	/**
	 * Compares two states and returns whether their parameters are equal.
	 *
	 * @param s1 First State
	 * @param s2 Second State
	 * @return true, if both are equal, otherwise false
	 */
	static bool statesAreEqual(RL_State s1, RL_State s2);


	/**
	 * Compares the state to another state and returns whether their parameters are equal.
	 *
	 * @param s The state to compare with
	 * @return true, if both are equal, otherwise false
	 */
	bool isEqualToState(RL_State s) const;


	/**
	 * The values of all the parameters of this state are checked whether
	 * they are within the range of their ParameterSpace, defined in the model.
	 * If they are outside this range, they are automatically corrected.
	 *
	 * @param model The Model with all the Parameter Spaces
	 */
	void correctValuesByModel(RL_Model* model);


	/**
	 * Prints all the parameters of the State
	 */
	void print() const;
};




#endif /* RLSTATE_H_ */
