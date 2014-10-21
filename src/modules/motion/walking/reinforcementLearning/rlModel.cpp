/*
 * rlmodel.cpp
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */

#include "rlModel.h"
#include "debug.h"


using namespace std;



RL_Model::RL_Model()
: numberOfStates(0)
, numberOfActions(0)
{
	// Empty!
}



/*------------------------------------------------------------------------------------------------*/


void RL_Model::addStateParameter(string name, int defaultValue, int minValue, int maxValue, int radialDistance) {
	// Add new parameter only if it does not exist yet.
	if (stateSpace.find(name) != stateSpace.end()) { return; }

	int range = 1 + maxValue - minValue;
	stateSpace[name] = {defaultValue, minValue, maxValue, range, radialDistance};

	if (numberOfStates == 0) {
		numberOfStates = stateSpace[name].range;
	} else {
		numberOfStates *= stateSpace[name].range;
	}

	// We need the new index shifts to be able to compute the new actions
	indexShiftsStates = computeIndexShiftsStates();
}


/*------------------------------------------------------------------------------------------------*/


void RL_Model::addActionParameter(string name, int defaultValue, int minValue, int maxValue, float impact) {
	// Add new parameter only if it does not exist yet.
	if (actionSpace.find(name) != actionSpace.end()) { return; }

	int range = 1 + maxValue - minValue;
	actionSpace[name] = {defaultValue, minValue, maxValue, range, impact};

	if (numberOfActions == 0) {
		numberOfActions = actionSpace[name].range;
	} else {
		numberOfActions *= actionSpace[name].range;
	}

	// We need the new index shifts to be able to compute the new actions
	indexShiftsActions = computeIndexShiftsActions();
	actions = computeAllActions();
}


/*------------------------------------------------------------------------------------------------*/


int RL_Model::getIndexForAction(const RL_Action& action) const {
	unsigned int index = 0;
	for (auto it(action.parameters.begin()); it != action.parameters.end(); ++it) {
		int localPos = 0;
		if (actionSpace.count(it->first)) {
			localPos = it->second - actionSpace.at(it->first).minValue;
		} else {
			printf("Model::getIndexForAction(): No such action-parameter: %s\n", it->first.c_str());
		}

		if (indexShiftsActions.count(it->first)) {
			index += localPos * indexShiftsActions.at(it->first);
		} else {
			printf("Model::getIndexForAction(): No such indexShift: %s\n", it->first.c_str());
		}
	}
	return index;
}


/*------------------------------------------------------------------------------------------------*/


int RL_Model::getIndexForState(const RL_State& state) const {
	unsigned int index = 0;
	for (auto it(state.parameters.begin()); it != state.parameters.end(); ++it) {
		int localPos = 0;
		if (stateSpace.count(it->first)) {
			localPos = it->second - stateSpace.at(it->first).minValue;
		} else {
			printf("Model::getIndexForState(): No such state-parameter: %s\n", it->first.c_str());
		}
		if (indexShiftsStates.count(it->first)) {
			index += localPos * indexShiftsStates.at(it->first);
		} else {
			printf("Model::getIndexForState(): No such indexShift: %s\n", it->first.c_str());
		}
	}
	return index;
}



/*------------------------------------------------------------------------------------------------*/


bool RL_Model::stateIndexIsValid(int index) const {
	if (index >= 0 && index < int(numberOfStates)) {
		return true;
	} else {
		return false;
	}
}


/*------------------------------------------------------------------------------------------------*/


bool RL_Model::actionIndexIsValid(int index) const {
//	for (auto it(actionSpace.rbegin()); it != actionSpace.rend(); ++it) {
//		int parameterValue = index / indexShiftsActions.at(it->first);
//		int value = parameterValue + it->second.minValue;
//		if ((value < it->second.minValue) || (value > it->second.maxValue)) return false;
//		index -= parameterValue * indexShiftsActions.at(it->first);
//	}
	RL_Action action = getActionForIndex(index);
	for (auto it(actionSpace.rbegin()); it != actionSpace.rend(); ++it) {
		if ((action.parameters[it->first] < it->second.minValue) || (action.parameters[it->first] > it->second.maxValue)) return false;
	}
	return true;
}



/*------------------------------------------------------------------------------------------------*/


RL_State RL_Model::getStateForIndex(int index) const {
	RL_State state;
	for (auto it(stateSpace.rbegin()); it != stateSpace.rend(); ++it) {
		if (indexShiftsStates.count(it->first)) {
			int parameterValue = index / int(indexShiftsStates.at(it->first));
			state.parameters[it->first] = parameterValue + it->second.minValue;
			index -= parameterValue * indexShiftsStates.at(it->first);
		} else {
			printf("Model::getStateForIndex(%d): No such state-parameter: %s\n", index, it->first.c_str());
		}
	}
	return state;
}


/*------------------------------------------------------------------------------------------------*/


RL_Action RL_Model::getActionForIndex(int index) const {
	RL_Action action;
	for (auto it(actionSpace.rbegin()); it != actionSpace.rend(); ++it) {
		if (indexShiftsActions.count(it->first)) {
			int parameterValue = index / int(indexShiftsActions.at(it->first));
			action.parameters[it->first] = parameterValue + it->second.minValue;
			index -= parameterValue * indexShiftsActions.at(it->first);
		} else {
			printf("Model::getActionForIndex(%d): No such action-parameter: %s\n", index, it->first.c_str());
		}
	}
	return action;
}


/*------------------------------------------------------------------------------------------------*/


RL_State RL_Model::getRandomState() const {
	RL_State tmpState;
	for (auto it(stateSpace.begin()); it != stateSpace.end(); ++it) {
		tmpState.parameters[it->first] = it->second.minValue + rand() % it->second.range;
	}
	return tmpState;
}


/*------------------------------------------------------------------------------------------------*/


RL_Action RL_Model::getRandomAction() const {
	RL_Action tmpAction;
	for (auto it(actionSpace.begin()); it != actionSpace.end(); ++it) {
		tmpAction.parameters[it->first] = it->second.minValue + rand() % it->second.range;
	}
	return tmpAction;
}


/*------------------------------------------------------------------------------------------------*/


int RL_Model::getRandomStateIndex() const {
	return getIndexForState(getRandomState());
}


/*------------------------------------------------------------------------------------------------*/


int RL_Model::getRandomActionIndex() const {
	return getIndexForAction(getRandomAction());
}


/*------------------------------------------------------------------------------------------------*/


RL_State RL_Model::getDefaultState() const {
	RL_State tmpState;
	for (auto it(stateSpace.begin()); it != stateSpace.end(); ++it) {
		tmpState.parameters[it->first] = it->second.defaultValue;
	}
	return tmpState;
}


/*------------------------------------------------------------------------------------------------*/


RL_Action RL_Model::getDefaultAction() const {
	RL_Action action;
	for (auto it(actionSpace.begin()); it != actionSpace.end(); ++it) {
		action.parameters[it->first] = it->second.defaultValue;
	}
	return action;
}


/*------------------------------------------------------------------------------------------------*/


RL_State RL_Model::getZeroState() const {
	RL_State tmpState;
	for (auto it(stateSpace.begin()); it != stateSpace.end(); ++it) {
		tmpState.parameters[it->first] = 0;
	}
	return tmpState;
}


/*------------------------------------------------------------------------------------------------*/


RL_Action RL_Model::getZeroAction() const {
	RL_Action tmpAction;
	for (auto it(actionSpace.begin()); it != actionSpace.end(); ++it) {
		tmpAction.parameters[it->first] = 0;
	}
	return tmpAction;
}

/*------------------------------------------------------------------------------------------------*/


int RL_Model::getDefaultStateIndex() const {
	return getIndexForState(getDefaultState());
}


/*------------------------------------------------------------------------------------------------*/


int RL_Model::getDefaultActionIndex() const {
	return getIndexForAction(getDefaultAction());
}


/*------------------------------------------------------------------------------------------------*/


map<string, int> RL_Model::computeIndexShiftsStates() {
	map<string, int> possibleCombinations;
	int possibilities = 1;
	for (auto it(stateSpace.begin()); it != stateSpace.end(); ++it) {
		possibleCombinations[it->first] = possibilities;
		possibilities *= stateSpace[it->first].range;
	}
	return possibleCombinations;
}


/*------------------------------------------------------------------------------------------------*/


map<string, int> RL_Model::computeIndexShiftsActions() {
	map<string, int> possibleCombinations;
	int possibilities = 1;
	for (auto it(actionSpace.begin()); it != actionSpace.end(); ++it) {
		possibleCombinations[it->first] = possibilities;
		possibilities *= actionSpace[it->first].range;
	}
	return possibleCombinations;
}


/*------------------------------------------------------------------------------------------------*/


double RL_Model::getImpact(std::string actionParameterName) {
	if (actionSpace.count(actionParameterName)) {
		return actionSpace[actionParameterName].impact;
	} else {
		return 0.0;
	}
}


/*------------------------------------------------------------------------------------------------*/


std::map<int, RL_Action> RL_Model::computeAllActions() {
	std::map<int, RL_Action> actions;
	for (int i = 0; i < int(numberOfActions); i++) {
		actions[i] = getActionForIndex(i);
	}
	return actions;
}


/*------------------------------------------------------------------------------------------------*/

