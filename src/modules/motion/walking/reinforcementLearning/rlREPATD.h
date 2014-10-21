/*
 * rlREPATD.h
 *
 *  Created on: Mar 9, 2013
 *      Author: stepuh
 */


#ifndef RLREPATD_H_
#define RLREPATD_H_

#include <atomic>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <set>
#include <thread>


#include "rlModel.h"
#include "rlQualityStorage.h"
#include "tools/concurrency/threadSafeQueue.h"

#include "messages/msg_qLearning.pb.h"
#include "messages/msg_repatd.pb.h"


struct RadialExpansionJob {
	//{s, a, targetQ, 1}
	int stateIndex;
	int actionIndex;
	float targetQ;
	float traceDiscount;
};



struct RadialExpansionEntry {
	float distanceDiscount;
	int diffIndex;
};




class RL_REPATD {

public:

	RL_Model* model;


	// Loading / Saving
	std::atomic<bool> isSaving;        // Indicates if the current model is being saved
	std::atomic<bool> isLoading;       // Indicates if the current model is being loaded
	bool loadingSucceeded;


	/**
	 * Default Constructor.
	 */
	RL_REPATD();


	/**
	 * Destructor.
	 */
	~RL_REPATD();


	/**
	 * Initialized an already existing REPATD-Object.
	 * The model includes state space and action space
	 *
	 * @param _model Model of the State Spaces and Actions
	 * @param _dicountFactor Discount Factor
	 * @param _traceDiscount How much less the next state in the eligibility trace is being influenced
	 * @param _learningRate Running average - how much impact new information has in relation to old information
	 * @param _eligibilityTraceLength How many steps are being stored in the eligibility trace
	 * @param _maxRadialDistance Maximal distance to perform the radial expansion for.
	 */
	void init(RL_Model* model, float _discountFactor, float _traceDecayRate, float _learningRate, int _eligibilityTraceLength);


	/**
	 * @param s
	 * @return
	 */
	float getBestQValueForState(int s) const;

	/**
	 * This method delivers the best action for a given state by using
	 * the Q-Function, calculated by the QLearning algorithm.
	 * Exploitation of the known world only. No exploration.
	 *
	 * @param s State to get the best action for
	 * @return Best known action
	 */
	RL_Action getBestActionForState(const RL_State& s, bool currentlyLearning) const;

	int getBestActionIndexForState(int s, bool currentlyLearning) const;


	/**
	 * This method delivers with probability (1-epsilon) the best action
	 * for a given state by using the Q-Function, calculated by the QLearning algorithm.
	 * With probability epsilon it chooses a random action.
	 * Exploitation of the known world and also exploration with probability epsilon.
	 *
	 * @param s State to get best the action for
	 * @param epsilon Probability to choose a random action, with epsilon in [0, 1]
	 * @return Best known action with probability (1-epsilon), random action with probability epsilon
	 */
	RL_Action getEGreedyActionForState(const RL_State& s, float epsilon) const;


	/**
	 * By receiving a reward for a current state, the Q-Value for the previous state is being updated.
	 * The last visited state (predecessor of current state) is pushed in the eligibility trace queue.
	 * Then all states in this queue are performing a radial expansion with their Q-Value, with
	 * decreasing importance depending on the distance within the radial expansion and depending on
	 * the distance within the eligibility trace.
	 *
	 * @param sample An experience, containing the previous state, the current state and the reward.
	 */
	void updateBySample(const RL_Sample& sample);


	/**
	 * Causes all States stored in the eligibility trace to perform a radial expansion,
	 * with a given value and a maximal distance of influence.
	 *
	 * @param targetQ The target Value that is being propagated.
	 */
	void propagateThroughEligibilityTrace(int s, int a, float deltaQ);


	/**
	 * Performs a radial expansion for a given state.
	 * By using the BFS-Algorithm, the target-Q-Value is being passed to the state's neighbors
	 * with decreasing importance depending on the distance.
	 *
	 * @param targetQ Value for the Q-Function that the states should converge to.
	 * @param rootState The initial state the radial expansion is starting from
	 * @param traceDiscount How much less the next state in the eligibility trace is being influenced
	 */
	void performRadialExpansion(int state, int action, float targetQ,  float traceDiscount);


	/**
	 * A Q-Value for a given state is being updated, depending on the distance to the center of the
	 * radial expansion on depending on the distance within the eligibility trace.
	 * The state's current Q-Value is a weighted average of experiences.
	 *
	 * @param state The state the Q-Value is updated for
	 * @param targetQ The Value the state's Q-Value is converging to
	 * @param distanceDiscount Distance within the radial expansion
	 * @param traceDiscount Distance within the eligibility trace
	 */
	void updateQ(int state, int action, float targetQ, float distanceDiscount, float traceDiscount);


	/**
	 * Computes the differences between the parameter ranges
	 * and the ratios between them. This is important to be able to compute
	 * the actual euclidean distance with different granularities in each parameter.
	 *
	 * @return Ratios between the parameter ranges.
	 */
	std::map<std::string, float> computeGranularityRatios();


	/**
	 * Calculates the weighted euclidean distance.
	 * The weights are the granularity ratios.
	 *
	 * @param s1 First State
	 * @param s2 Second State
	 * @return Weighted euclidean distance.
	 */
	float getDistance(const RL_State& s1, const RL_State& s2) const;


	/**
	 * Calculates the weighted euclidean distance.
	 * The weights are the granularity ratios.
	 *
	 * @param s1 First State
	 * @param s2 Second State
	 * @return Weighted euclidean distance.
	 */
	float getDistance(int s1, int s2) const;


	bool isInRadialDistance(const RL_State& s1, const RL_State& s2) const;
	bool isInRadialDistance(int s1, int s2) const;


	/**
	 * Computes the radial base function for a given mean and a given standard deviation.
	 *
	 * @param x Mean
	 * @param standardDeviation Standard Deviation
	 * @return Radial Base Value
	 */
	float radial(float x, float standardDeviation);


	/**
	 * Pre-computes the radial expansion of the experience.
	 * Because the borders of the state-space must not be trespassed, the
	 * radial expansion is stored in a spanning tree structure.
	 * Here the nodes contain the relative indices and the radial discounter.
	 * Later in performRadialExpansion(): If a inner node is on the border,
	 * the whole branch is being ignored.
	 *
	 * @return Radial Expansion stored in a spanning tree.
	 */
	std::vector<RadialExpansionEntry> computeRadialExpansion();


	/**
	 * If we already have an experience for the state at the given index, we return
	 * the Q-Value. Otherwise return 0.
	 *
	 * @param state The index of the state of which we want to know the Q-Value
	 * @return Q-Value of the state
	 */
	float getQValue(int s, int a) const;


	/**
	 * Saves the model and all learned information into a Protobuf Message.
	 * This includes Q, e and stateActionCounter.
	 * This calls saveWhatWasLearned in a second thread
	 * @see saveWhatWasLearned
	 *
	 * @param fileName Path/Name of the Protobuf Message
	 */
	void saveWhatWasLearnedAsync(std::string fileName);


	/**
	 * Saves the model and all learned information into a Protobuf Message.
	 * This includes Q, e and stateActionCounter.
	 * This calls loadWhatWasLearned in a second thread
	 * @see loadWhatWasLearned
	 *
	 * @param fileName Path/Name of the Protobuf Message
	 */
	void loadWhatWasLearnedAsync(std::string fileName);


	bool isInitialized() {
		return initialized.load();
	}


private:
	void worker();


	/**
	 * Loads the model and all learned information from a Protobuf Message.
	 * Thereby model, Q, e and stateActionCounter are being overwritten.
	 *
	 * @param fileName Path/Name of the Protobuf Message
	 * @return true iff model was loaded (and initialized) successfully
	 */
	bool loadWhatWasLearned(std::string fileName);


	/**
	 * Loads the model and all learned information from a Protobuf Message.
	 * Thereby model, Q, e and stateActionCounter are being overwritten.
	 * If loading fails, try to load the backup file.
	 *
	 * @param fileName Path/Name of the Protobuf Message
	 * @return true iff model was loaded (and initialized) successfully
	 */
	bool loadWhatWasLearnedOrBackup(std::string fileName);


	/**
	 * Saves the model and all learned information into a Protobuf Message.
	 * This includes Q, e and stateActionCounter.
	 *
	 * @param fileName Path/Name of the Protobuf Message
	 */
	void saveWhatWasLearned(std::string fileName);


protected:


	float discountRate;
	float learningRate;

	RLQualityStorage Q;  // Quality of a state.


	// Eligibility Traces
	std::list<std::pair <int, int>> eligibilityTrace;           // Queue of last visited <state_index, action_index>- pairs
	int traceLength;                                            // Max number of States that we want to remember
	float traceDecayRate;                                       // Rate of which the entries in the eligibility-list are getting smaller


	// Radial Expansion
	std::vector<RadialExpansionEntry> radialExpansionList;
	std::map<std::string, float> granularityRatios;             // We remember this for fast distance-calculation


	// Worker Thread
	mutable CriticalSection cs;
	mutable CriticalSection notWorking;
	int jobQueueLength;
	ThreadSafeQueue<RadialExpansionJob> jobQueue;               // Queue of all the radial expansions that are jet to perform

	std::thread workerThread;
	std::atomic<bool> workerAtWork;

	std::atomic<bool> initialized;
	std::atomic<bool> isRunning;
};



#endif /* RLREPATD_H_ */
