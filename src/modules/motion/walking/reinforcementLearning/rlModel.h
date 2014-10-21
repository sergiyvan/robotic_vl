/*
 * rlmodel.h
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */

#include "rlState.h"

#ifndef RLMODEL_H_
#define RLMODEL_H_

#include <map>
#include <string>
#include <vector>




/**
 * We use the technique of reinforcement leaning to optimize the walker.
 * The basis of this technique is an agent (in our case the robot) and an environment the agent is acting in.
 * The fundamental idea is trial and error. While getting a reward for performing an action
 * in a certain state, the agent then ends up in a successor state. The final goal is to figure out
 * a policy that provides the agent with a strategy on what action to perform in a certain state in order
 * to maximize the total reward.
 * In reinforcement learning we approximate the reality with a model that mainly consists of a state space,
 * an action space, the transition-probability-distribution and the reward-probability-distribution.
 */



/**
 * A state parameter is a variable over which we want to learn.
 */
struct RL_StateParameter {
	int defaultValue;
	int minValue;
	int maxValue;
	int range;
	int radialDistance;
};


/**
 * An action parameter is a variable that can be effected by the learning algorithm.
 */
struct RL_ActionParameter {
	int defaultValue;
	int minValue;
	int maxValue;
	int range;
	float impact;
};


/**
 *  A sample is an experience the agent uses to learn.
 */
struct RL_Sample {
	RL_State state;
	RL_Action action;
	float reward;
	RL_State succState;
};





class RL_Model {
public:

	std::map<std::string, RL_StateParameter> stateSpace;
	std::map<std::string, RL_ActionParameter> actionSpace;

	unsigned int numberOfStates;
	unsigned int numberOfActions;

	std::map<int, RL_Action> actions;                 // We remember the indices of all actions
	std::map <std::string, int> indexShiftsActions;   // What to add/subtract to a state's index, when changing a certain parameter.
	std::map <std::string, int> indexShiftsStates;    // What to add/subtract to a state's index, when changing a certain parameter.


	RL_Model();



	/**
	 * Adds a new StateParameter to the Model. All new possible states are automatically inferred.
	 * The states are sorted lexicographically which makes it much easier to locate them later.
	 *
	 * @param name Key of the new ParameterSpace
	 * @param defaultValue Default value of the parameter if none is given
	 * @param minValue Minimal value of the parameter
	 * @param maxValue Maximal value of the parameter
	 */
	void addStateParameter(std::string name, int defaultValue, int minValue, int maxValue, int radialDistance);


	/**
	 * Adds a new ActionParameter to the Model. All new possible actions are automatically inferred.
	 * The actions are sorted lexicographically which makes it much easier to locate them later.
	 *
	 * @param name Key of the new ParameterSpace
	 * @param defaultValue Default value of the parameter if none is given
	 * @param minValue Minimal value of the parameter
	 * @param maxValue Maximal value of the parameter
	 * @param impact Value multiplied with the current value of the action parameter
	 */
	void addActionParameter(std::string name, int defaultValue, int minValue, int maxValue, float impact);


	/**
	 * This method computes the index for a given State-Object.
	 * This index is basically an integer-value, representing all state-parameters.
	 * Complexity: O(m), where m = number of parameters in the State
	 *
	 * @param state State to find the index for
	 * @return The index of the state.
	 */
	int getIndexForState(const RL_State& state) const;


	/**
	 * This method computes the index for a given Action-Object.
	 * This index is basically an integer-value, representing all action-parameters.
	 * Complexity: O(m), where m = number of parameters in Action a
	 *
	 * @param a Action to find the index for
	 * @return The index of the action.
	 */
	int getIndexForAction(const RL_Action& action) const;


	/**
	 * Returns true, if the parameters, represented by the given index are
	 * within the bounds of the parameter space. Returns false, if it is outside.
	 *
	 * @param index An index representing a state
	 * @return Whether or not the index are within the bounds of the parameter space
	 */
	bool stateIndexIsValid(int index) const;


	/**
	 * Returns true, if the parameters, represented by the given index are
	 * within the bounds of the parameter space. Returns false, if it is outside.
	 *
	 * @param index An index representing an action
	 * @return Whether or not the index are within the bounds of the parameter space
	 */
	bool actionIndexIsValid(int index) const;


	/**
	 * Returns the impact for the name of a parameterspace
	 * @param parameterSpaceName Name of parameterspace
	 *
	 * @return Impact
	 */
	double getImpact(std::string actionParameterName);


	/**
	 * Converts a given Index into a State-Object.
	 *
	 * @param index Index of the state, the method creates the object for.
	 * @return Object for this index
	 */
	RL_State getStateForIndex(int index) const;


	/**
	 * Converts a given Index into an Action-Object.
	 *
	 * @param index Index of the action, the method creates the object for.
	 * @return Object for this index
	 */
	RL_Action getActionForIndex(int index) const;


	/**
	 * Returns a random State-Object.
	 *
	 * @return Random State-Object.
	 */
	RL_State getRandomState() const;


	/**
	 * Returns a random Action-Object.
	 *
	 * @return Random Action-Object.
	 */
	RL_Action getRandomAction() const;


	/**
	 * Returns the index of a random State.
	 *
	 * @return Random State-Index.
	 */
	int getRandomStateIndex() const;


	/**
	 * Returns the index of a random Action.
	 *
	 * @return Random Action-Index.
	 */
	int getRandomActionIndex() const;


	/**
	 * Returns a state whose parameters are all set to default value.
	 *
	 * @return State with default parameter values.
	 */
	RL_State getDefaultState() const;


	/**
	 * Returns action where all entries are set to default value.
	 *
	 * @return Default Action-Object
	 */
	RL_Action getDefaultAction() const;


	/**
	 * Returns a state whose parameters are all set zero.
	 * Warning!! This State might not exist because it could
	 * lie outside the bounds defined by minValue / maxValue of the stateSpace.
	 *
	 * @return State with all parameters set to zero.
	 */
	RL_State getZeroState() const;


	/**
	 * Returns an action whose parameters are all set zero.
	 * Warning!! This Action might not exist because it could
	 * lie outside the bounds defined by minValue / maxValue of the actionSpace.
	 *
	 * @return Action with all parameters set to zero.
	 */
	RL_Action getZeroAction() const;


	/**
	 * Returns the index of the default State.
	 *
	 * @return Default State-Index.
	 */
	int getDefaultStateIndex() const;


	/**
	 * Returns the index of the default Action.
	 *
	 * @return Default Action-Index.
	 */
	int getDefaultActionIndex() const;



private:


	/**
	 * Helper function.
	 * Computes for each parameter what to add/subtract to a state's index,
	 * when changing this parameter.
	 *
	 * @return List of Values that are added to a state's index when changing a certain parameter
	 */
	std::map<std::string, int> computeIndexShiftsStates();


	/**
	 * Helper function.
	 * Computes for each parameter what to add/subtract to a action's index,
	 * when changing this parameter.
	 *
	 * @return List of Values that are added to a state's index when changing a certain parameter
	 */
	std::map<std::string, int> computeIndexShiftsActions();


	/**
	 * Computes a list (map) of all possible actions
	 * @return
	 */
	std::map<int, RL_Action> computeAllActions();


};



#endif /* RLMODEL_H_ */
