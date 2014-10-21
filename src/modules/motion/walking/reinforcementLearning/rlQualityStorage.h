/*
 * rlQualityStorage.h
 *
 *  Created on: Mar 23, 2014
 *      Author: dseifert
 */

#ifndef RLQUALITYSTORAGE_H_
#define RLQUALITYSTORAGE_H_

#include "debug.h"

#include <vector>

/*------------------------------------------------------------------------------------------------*/

struct ActionsForState {
	// constructor
	ActionsForState(int numberOfActions)
		: numberOfActions(numberOfActions)
	{
		// reserve 10 initial values set to zero
		actionValues.resize(numberOfActions, 0);
		hasLearned = false;
	}

	const float& operator[](unsigned index) const {
		return actionValues[index];
	}

	void setAction(int action, float value) {
		ASSERT((int)actionValues.size() == numberOfActions);
		actionValues[action] = value;
		hasLearned = true;
	}

	// the action values
	std::vector<float> actionValues;

	bool hasLearned;
	int numberOfActions;
};


/*------------------------------------------------------------------------------------------------*/

class RLQualityStorage {
public:
	RLQualityStorage()
		: numberOfActions(0)
	{}

	void init(const RL_Model* model) {
		init(model->numberOfStates, model->numberOfActions);
	}

	void init(unsigned int  expectedStorageCapacity, unsigned int numberOfActions) {
		storage.clear();
		this->numberOfActions = numberOfActions;
		if (expectedStorageCapacity > 0)
			storage.resize(expectedStorageCapacity, numberOfActions);
	}

	size_t size() const {
		return storage.size();
	}

	const ActionsForState& operator[](unsigned index) const {
		return storage[index];
	}

	ActionsForState& operator[](unsigned index) {
		if (index >= storage.size()) {
			storage.resize(index+1, numberOfActions);
		}

		return storage[index];
	}

	inline unsigned int getNumberOfActionsPerState() const {
		return numberOfActions;
	}

private:
	std::vector<ActionsForState> storage;
	unsigned int numberOfActions;
};


#endif /* RLQUALITYSTORAGE_H_ */
