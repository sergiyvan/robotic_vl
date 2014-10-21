/*
 * rlREPATD.h
 *
 *  Created on: Mar 9, 2013
 *      Author: stepuh
 */


#ifndef NEURALLEARNING_H_
#define NEURALLEARNING_H_

#include <atomic>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <set>


#include "rlModel.h"
#include "tools/concurrency/threadSafeQueue.h"

#include "messages/msg_repatd.pb.h"

#include "modules/motion/walking/supervisedLearning/slNeuronalNetwork.h"



struct NeuralJob {
	RL_State state;
	RL_Action action;
	float targetQ;
	float traceDiscount;
};




class NeuralLearning {

public:

	RL_Model* model;                                             // Set of States and Set of Actions

	float discountRate;
	float learningRate;


	std::vector<SL_NeuronalNetwork*> Q_ANNs;                     // Approximation of Action-Value Function Q

	// Eligibility Traces
	std::list<std::pair <RL_State, RL_Action>> eligibilityTrace; // Queue of last visited <state_index, action_index>- pairs
	int traceLength;                                             // Max number of States-Action-Pairs that we want to remember
	float traceDecayRate;                                        // Rate of which the entries in the eligibility-list are getting smaller


	std::atomic<bool> isSaving;                                  // Indicates whether the current model is being saved
	std::atomic<bool> isLoading;                                 // Indicates whether the current model is being loaded

	bool isRunning;

	mutable CriticalSection cs;
	int jobQueueLength;
	//ThreadSafeQueue<NeuralJob>* jobQueue;                        // Queue of all the radial expansions that are jet to perform
	std::queue<NeuralJob> jobQueue;



	/**
	 * Default Constructor.
	 */
	NeuralLearning();


	/**
	 * Destructor.
	 */
	~NeuralLearning();


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
	void init(RL_Model* _model, float _discountRate, float _traceDecayRate, float _learningRate, int _eligibilityTraceLength);


	/**
	 * This method delivers the best action's index for a given
	 * state's index by using the Q-Function, calculated by
	 * the QLearning algorithm.
	 * Exploitation of the known world only. No exploration.
	 *
	 * @param s State to get the best action for
	 * @return Best known action for state s
	 */
	RL_Action getBestActionForState(const RL_State& s) const;


	/**
	 * @param s State to get the best Q-Value for, by trying out all actions
	 * @return Best Q-Value
	 */
	float getBestQValueForState(const RL_State& s) const;


	/**
	 * This method delivers the best action for a given state by using
	 * the Q-Function, calculated by the QLearning algorithm.
	 * Exploitation of the known world only. No exploration.
	 *
	 * @param s State to get the best action for
	 * @return Best known action
	 */
	RL_Action getBestActionForState(const RL_State& s, bool currentlyLearning) const;


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
	void propagateThroughEligibilityTrace(const RL_State& s, const RL_Action& a, float deltaQ);


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
	void updateQ(const RL_State& s, const RL_Action& a, float targetQ, float traceDiscount);


	/**
	 * If we already have an experience for the state at the given index, we return
	 * the Q-Value. Otherwise return 0.
	 *
	 * @param state The index of the state of which we want to know the Q-Value
	 * @return Q-Value of the state
	 */
	float getQValue(const RL_State& s, const RL_Action& a) const;


	/**
	 * Teaches the neural network the last n experiences in a separate thread.
	 */
	void worker();


	/**
	 * Saves the model and all learned information into a Protobuf Message.
	 * This includes Q, e and stateActionCounter.
	 *
	 * @param fileName Path/Name of the Protobuf Message
	 */
	void saveWhatWasLearned(std::string fileName);


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
	 * This calls loadWhatWasLearned in a second thread
	 * @see loadWhatWasLearned
	 *
	 * @param fileName Path/Name of the Protobuf Message
	 */
	void loadWhatWasLearnedAsync(std::string fileName);


	arma::vec generatePattern(const RL_State& s, const RL_Action& a) const;


};



#endif /* RLREPATD_H_ */
