/*
 * rlREPATD.cpp
 *
 *  Created on: Mar 9, 2013
 *      Author: stepuh
 */

#include "rlREPATD.h"

#include <fstream>
#include <iostream>
//#include <thread>

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "utils/utils.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "tools/Clockwork/clockwork.h"

using namespace std;




/*------------------------------------------------------------------------------------------------*/


RL_REPATD::RL_REPATD()
	: model(nullptr)
	, isSaving(false)
	, isLoading(false)
	, loadingSucceeded(false)
	, discountRate(0)
	, learningRate(0)
	, traceLength(0)
	, traceDecayRate(0)
	, jobQueue(256)
	, workerAtWork(false)

	, initialized(false)
	, isRunning(true)
{
	initialized.store(false);
	isSaving.store(false);
	isLoading.store(false);
	isRunning.store(true);

	cs.setName("REPATD_WORKER");
	notWorking.setName("REPATD NOT WORKING");
}


/*------------------------------------------------------------------------------------------------*/


RL_REPATD::~RL_REPATD() {
	while (true == workerAtWork.load()) {
		std::this_thread::yield();
	}

	isRunning.store(false);
	isLoading.store(false);
	isSaving.store(false);
	initialized.store(false);

	if (workerThread.joinable())
		workerThread.join();

	printf("REPATD DONE.\n");
}



/*------------------------------------------------------------------------------------------------*/


void RL_REPATD::init(RL_Model* _model, float _discountRate, float _traceDecayRate, float _learningRate, int _traceLength) {
	model              = _model;
	discountRate       = _discountRate;
	traceDecayRate     = _traceDecayRate;
	traceLength        = _traceLength;
	learningRate       = _learningRate;

	granularityRatios  = computeGranularityRatios();
	radialExpansionList    = computeRadialExpansion();

	initialized.store(true);

	Q.init(model);

	printf("REPATD: INITIALIZED\n");

	workerThread = std::thread(&RL_REPATD::worker, this);
}


/*------------------------------------------------------------------------------------------------*/


// Compute: max_a : Q(s, a)
float RL_REPATD::getBestQValueForState(int s) const {
	CriticalSectionLock lock2(notWorking);

	// return default value if we are currently loading
	if (true == isLoading.load())
		return 0.;

	CriticalSectionLock lock(cs);

	if (false == Q[s].hasLearned)
		return 0;

	float bestValue = -INFINITY;

	// Try out each possible action
	for (auto it(model->actions.begin()); it != model->actions.end(); ++it) {
		float tmpValue = getQValue(s, it->first);

		if (tmpValue > bestValue) {
			bestValue = tmpValue;
		}
	}

	return bestValue;
}


/*------------------------------------------------------------------------------------------------*/


// Compute: argmax_a : Q(s, a)
int RL_REPATD::getBestActionIndexForState(int s, bool currentlyLearning) const {
	CriticalSectionLock lock2(notWorking);

	// return default if we are loading
	if (true == isLoading.load())
		return model->getDefaultActionIndex();

	CriticalSectionLock lock(cs);

	// If there is no experience yet
	if (false == Q[s].hasLearned) {
		if (currentlyLearning) {
			return model->getRandomActionIndex();
		} else {
			return model->getDefaultActionIndex();
		}
	}

	// Pick best of all the available actions (those that have already been tried out)
	float bestValue = -INFINITY;
	int bestAction = model->getRandomActionIndex();

	for (auto it(model->actions.begin()); it != model->actions.end(); ++it) {
		float tmpValue = getQValue(s, it->first);
		if (tmpValue > bestValue) {
			bestValue = tmpValue;
			bestAction = it->first;
		}
	}

	if (bestValue == 0) return model->getRandomActionIndex();

	return bestAction;
}


/*------------------------------------------------------------------------------------------------*/


RL_Action RL_REPATD::getBestActionForState(const RL_State& s, bool currentlyLearning) const {
	int s_index = model->getIndexForState(s);
	int bestAction = getBestActionIndexForState(s_index, currentlyLearning);
	return model->getActionForIndex(bestAction);
}


/*------------------------------------------------------------------------------------------------*/


RL_Action RL_REPATD::getEGreedyActionForState(const RL_State& s, float epsilon) const {
	epsilon *= 1000;
	float random = rand() % 1000;
	if (random > epsilon) {
		return getBestActionForState(s, true); // if we use e-greedy, we learn -> so currentlyLearning is set true
	} else {
		return model->getRandomAction();
	}
}


/*------------------------------------------------------------------------------------------------*/


void RL_REPATD::updateBySample(const RL_Sample& sample) {
	CriticalSectionLock lock(notWorking);

	// Don't update if current model is being saved
	if (true == isSaving.load())
		return;

	int s      = model->getIndexForState(sample.state);
	int s_succ = model->getIndexForState(sample.succState);
	int a      = model->getIndexForAction(sample.action);

	float targetQ = sample.reward + discountRate * getBestQValueForState(s_succ);
	propagateThroughEligibilityTrace(s, a, targetQ);
}


/*------------------------------------------------------------------------------------------------*/



void RL_REPATD::propagateThroughEligibilityTrace(int s, int a, float targetQ) {
	// at (s, a) the eligibility trace is set to 1
	jobQueue.enqueue({s, a, targetQ, 1.0});

	float traceDiscount = 1.0;
	for (auto it(eligibilityTrace.begin()); it != eligibilityTrace.end(); ++it) {
		std::pair<int, int> pair = *it;
		int tmp_s = pair.first;
		int tmp_a = pair.second;

		// at every pair (s', a') where s' == s the eligibility trace is set to 0
		//if (tmp_s != s) {
			jobQueue.enqueue({tmp_s, tmp_a, targetQ, traceDiscount});
			traceDiscount =  traceDiscount * traceDecayRate * discountRate;
		//} else {
			// erase while iterating - this is a valid method to do this. if you don't believe me: look it up yourself!
		//	it = eligibilityTrace.erase(it);
		//}
	}

	// Push this experience in eligibility trace
	eligibilityTrace.push_front(std::pair<int, int>(s, a));

	// If eligibility trace got too long, we cut off the last part
	if (int(eligibilityTrace.size()) > traceLength) {
		eligibilityTrace.pop_back();
	}
}



/*------------------------------------------------------------------------------------------------*/



void RL_REPATD::performRadialExpansion(int state, int action, float targetQ,  float traceDiscount) {
	for (unsigned int i = 0; i < radialExpansionList.size(); i++) {
		int neighborStateIndex = state + radialExpansionList[i].diffIndex;
		if (isInRadialDistance(state, neighborStateIndex)) {
			if (model->stateIndexIsValid(neighborStateIndex)) {
				updateQ(neighborStateIndex, action, targetQ, radialExpansionList[i].distanceDiscount, traceDiscount);
			}
		}
	}
}



/*------------------------------------------------------------------------------------------------*/



void RL_REPATD::updateQ(int state, int action, float targetQ, float distanceDiscount, float traceDiscount) {
	CriticalSectionLock lock(notWorking);

	if (true == isLoading.load() || true == isSaving.load())
		return;

	float qValue = getQValue(state, action);
	float error = (targetQ - qValue);
	float new_value = qValue + (learningRate * distanceDiscount * traceDiscount * error);

	if (new_value != INFINITY && new_value != -INFINITY && new_value != NAN) {
		CriticalSectionLock lock(cs);
		Q[state].setAction(action, new_value);
	}
}



/*------------------------------------------------------------------------------------------------*/



map<string, float> RL_REPATD::computeGranularityRatios() {
	// Find parameter space with greatest range
	int bestGranularity = 0;
	for (auto it(model->stateSpace.begin()); it != model->stateSpace.end(); it++) {
		if (it->second.range > bestGranularity) {
			bestGranularity = it->second.range;
		}
	}

	// Use best granularity to figure out the ratios of all the other parameter spaces.
	map<string, float> ratios;
	for (auto it(model->stateSpace.begin()); it != model->stateSpace.end(); it++) {
		ratios[it->first] = float(bestGranularity) / float(it->second.range);
	}

	return ratios;
}



/*------------------------------------------------------------------------------------------------*/



// Here we calculate the weighted euclidean distance.
// The weights are the granularity ratios.
float RL_REPATD::getDistance(const RL_State& s1, const RL_State& s2) const {
	float distance = 0;

	for (auto it(s1.parameters.begin()); it != s1.parameters.end(); it++) {
		if (granularityRatios.count(it->first) && s2.parameters.count(it->first)) {
			float tmpDistance = int(it->second) - int(s2.parameters.at(it->first));
			tmpDistance *= granularityRatios.at(it->first);
			tmpDistance *= tmpDistance;
			distance += tmpDistance;
		} else {
			printf("REPATD::getDistance(): No such state-parameter: %s\n", it->first.c_str());
		}
	}
	return sqrt(distance);
}



/*------------------------------------------------------------------------------------------------*/



float RL_REPATD::getDistance(int s1, int s2) const {
	RL_State state1 = model->getStateForIndex(s1);
	RL_State state2 = model->getStateForIndex(s2);
	return getDistance(state1, state2);
}



/*------------------------------------------------------------------------------------------------*/



bool RL_REPATD::isInRadialDistance(const RL_State& s1, const RL_State& s2) const {
	for (auto it(s1.parameters.begin()); it != s1.parameters.end(); it++) {
		int distance = std::abs(it->second - s2.parameters.at(it->first));
		if (distance > model->stateSpace.at(it->first).radialDistance) {
			return false;
		}
	}
	return true;
}



/*------------------------------------------------------------------------------------------------*/



bool RL_REPATD::isInRadialDistance(int s1, int s2) const {
	RL_State state1 = model->getStateForIndex(s1);
	RL_State state2 = model->getStateForIndex(s2);
	return isInRadialDistance(state1, state2);
}



/*------------------------------------------------------------------------------------------------*/



float RL_REPATD::radial(float x, float standardDeviation) {
	// value at standard deviation * 1: 0.6
	// value at standard deviation * 2: 0.13
	// value at standard deviation * 3: 0.01
	// -> maxDistance = 3x standard deviation (seems like a good choice)
	return exp((-0.5 * x*x) / (standardDeviation * standardDeviation));
}



/*------------------------------------------------------------------------------------------------*/



std::vector<RadialExpansionEntry> RL_REPATD::computeRadialExpansion() {
	std::vector<RadialExpansionEntry> radialExpansion;

	int maxRadialDistance = 0;   // needed for discount calculation

	// compute root sate index
	RL_State rState;
	for (auto it(model->stateSpace.begin()); it != model->stateSpace.end(); ++it) {
		rState.parameters[it->first] = int((it->second.maxValue + it->second.minValue) / 2);
	}
	int rootIndex = model->getIndexForState(rState);


	// Create clockwork
	std::vector<int> cw_minValues;
	std::vector<int> cw_maxValues;
	for (auto it(model->stateSpace.begin()); it != model->stateSpace.end(); ++it) {
		cw_minValues.push_back((-1) * it->second.radialDistance);
		cw_maxValues.push_back(it->second.radialDistance);

		if (maxRadialDistance < it->second.radialDistance) {
			maxRadialDistance = it->second.radialDistance;
		}
	}
	Clockwork *cw = new Clockwork(cw_minValues, cw_maxValues);
	cw->setToMinValue();


	RL_State rootState = model->getZeroState();
	RL_State tmpState;

	for (int i = 0; i < cw->getPossibleCombinations(); i++) {

		// apply current clockwork status to state parameters
		int tmp_n = 0;
		for (auto it(model->stateSpace.begin()); it != model->stateSpace.end(); ++it) {
			tmpState.parameters[it->first] = cw->getValue()[tmp_n];
			tmp_n++;
		}

		// use the state to compute entry for radial expansion
		RadialExpansionEntry entry;
		entry.diffIndex        = model->getIndexForState(tmpState) - rootIndex;
		entry.distanceDiscount = radial(getDistance(rootState, tmpState), maxRadialDistance / 3.0);
		radialExpansion.push_back(entry);

		cw->increment();
	}

	return radialExpansion;
}



/*------------------------------------------------------------------------------------------------*/



float RL_REPATD::getQValue(int s, int a) const {
	CriticalSectionLock lock(notWorking);

	if (true == isLoading.load())
		return 0.;

	if (Q[s].hasLearned) {
		return Q[s][a];
	} else {
		return 0.0;
	}
}



/*------------------------------------------------------------------------------------------------*/



void RL_REPATD::saveWhatWasLearnedAsync(string fileName) {
	{
		CriticalSectionLock lock(notWorking);
		bool expectedCurrentValue = false;
		if (!isSaving.compare_exchange_strong(expectedCurrentValue, true)) {
			ERROR("saveWhatWasLearned called while already saving");
			return; // Don't save if a saving routine is already running
		}
	}

	std::thread t(&RL_REPATD::saveWhatWasLearned, this, fileName);
	t.detach();
}



/*------------------------------------------------------------------------------------------------*/



void RL_REPATD::loadWhatWasLearnedAsync(string fileName) {
	bool expectedValueLoading = false;
	if (!isLoading.compare_exchange_strong(expectedValueLoading, true)) {
		ERROR("loadWhatWasLearnedAsync() called while already loading.");
		return;
	}

	// start detached thread to do the actual loading
	std::thread t(&RL_REPATD::loadWhatWasLearnedOrBackup, this, fileName);
	t.detach();
}



/*------------------------------------------------------------------------------------------------*/



bool RL_REPATD::loadWhatWasLearnedOrBackup(string fileName) {
	bool success = loadWhatWasLearned(fileName);

	if (false == success) {
		std::string backupFileName = fileName + ".bak";

		if (fileExists(backupFileName.c_str())) {
			WARNING("Trying to load backup file with walker parameters.");
			success = loadWhatWasLearned(backupFileName);
			if (success) {
				rename(backupFileName.c_str(), fileName.c_str());
				sync();
			}
		}
	}

	return success;
}



/*------------------------------------------------------------------------------------------------*/



void RL_REPATD::saveWhatWasLearned(string fileName) {
	printf("\nREPATD: NOW SAVING...\n");

	MSG_RL_REPATD repatd;
	repatd.set_discountrate(discountRate);
	repatd.set_learningrate(learningRate);
	repatd.set_tracedecayrate(traceDecayRate);
	repatd.set_tracelength(traceLength);


	MSG_RL_REPATD::Model *tmpModel = repatd.mutable_model();
	for (auto it(model->stateSpace.begin()); it != model->stateSpace.end(); ++it) {
		MSG_RL_REPATD::Model::StateParameter* tmpStateParameter = tmpModel->add_stateparameter();
		tmpStateParameter->set_name(it->first);
		tmpStateParameter->set_minvalue(it->second.minValue);
		tmpStateParameter->set_maxvalue(it->second.maxValue);
		tmpStateParameter->set_defaultvalue(it->second.defaultValue);
		tmpStateParameter->set_radialdistance(it->second.radialDistance);
	}

	for (auto it(model->actionSpace.begin()); it != model->actionSpace.end(); ++it) {
		MSG_RL_REPATD::Model::ActionParameter* tmpActionParameter = tmpModel->add_actionparameter();
		tmpActionParameter->set_name(it->first);
		tmpActionParameter->set_minvalue(it->second.minValue);
		tmpActionParameter->set_maxvalue(it->second.maxValue);
		tmpActionParameter->set_defaultvalue(it->second.defaultValue);
		tmpActionParameter->set_impact(it->second.impact);
	}


	std::stringstream outputStr;
	for (int stateIndex = 0; stateIndex < (int)Q.size(); stateIndex++) {
		if (Q[stateIndex].hasLearned) {
			outputStr.write("1", 1);
			for (unsigned int actionIndex = 0; actionIndex < Q.getNumberOfActionsPerState(); actionIndex++) {
				float value = Q[stateIndex][actionIndex];
				outputStr.write( reinterpret_cast< const char* >( &value ), sizeof(value) );
			}
		} else {
			outputStr.write("0", 1);
		}
	}
	repatd.mutable_q()->set_data(outputStr.str());

	if (!repatd.IsInitialized()) {
		printf("REPATD: Could not create ProtoBuf Message! Not initialized!\n");
	} else {
		printf("REPATD: Successfully created ProtoBuf Message.\n");
	}
	// create backup file
	std::string backupFileName = fileName + ".bak"; // backup copy
	if (fileExists(fileName.c_str())) {
		rename(fileName.c_str(), backupFileName.c_str());
		sync();
	}

	std::ofstream file(fileName, std::ios::out | std::ios::binary);
	if (false == file.fail()) {
		repatd.SerializeToOstream(&file);
		sync();
		printf("REPATD: Successfully serialized Protobuf-Message!\n");
	} else {
		printf("REPATD: Could not serialize Protobuf-Message!\n");
	}

	printf("REPATD: SAVING DONE\n");
	isSaving.store(false);
}



/*------------------------------------------------------------------------------------------------*/



bool RL_REPATD::loadWhatWasLearned(string fileName) {
	ASSERT(true == isLoading.load());
	ASSERT(true == isInitialized());

	robottime_t sysTime = getCurrentTime();

	loadingSucceeded = false;

	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (file.fail()) {
		ERROR("Could not open Protobuf-Message for REPATD!");
		isLoading.store(false);
		return false;
	}

	printf("REPATD: streaming protobuf message...\n");
	google::protobuf::io::IstreamInputStream inputStream(&file);
	google::protobuf::io::CodedInputStream codedIStream(&inputStream);
	codedIStream.SetTotalBytesLimit(512*1024*1024, -1);
printf("runtime --- open file:\t%.1f ms\n", (getCurrentTime()-sysTime).value());
sysTime = getCurrentTime();

	MSG_RL_REPATD repatd;
	repatd.ParseFromCodedStream(&codedIStream);
	if (!repatd.IsInitialized()) {
		ERROR("Could not parse Protobuf-Message for REPATD!");
		isLoading.store(false);
		return false;
	}
printf("runtime --- parse:\t%.1f ms\n", (getCurrentTime()-sysTime).value());
sysTime = getCurrentTime();

	robottime_t sysTime2 = getCurrentTime();
	printf("Time Preparation: %d\n", int(sysTime2.value() - sysTime.value()));


	RL_Model* tmpModel = new RL_Model;
	for (int ps = 0; ps < repatd.model().stateparameter().size(); ps++) {
		tmpModel->addStateParameter(repatd.model().stateparameter(ps).name(),
				                    repatd.model().stateparameter(ps).defaultvalue(),
				                    repatd.model().stateparameter(ps).minvalue(),
				                    repatd.model().stateparameter(ps).maxvalue(),
				                    repatd.model().stateparameter(ps).radialdistance());
	}

	for (int ps = 0; ps < repatd.model().actionparameter().size(); ps++) {
		tmpModel->addActionParameter(repatd.model().actionparameter(ps).name(),
				                     repatd.model().actionparameter(ps).defaultvalue(),
				                     repatd.model().actionparameter(ps).minvalue(),
				                     repatd.model().actionparameter(ps).maxvalue(),
				                     repatd.model().actionparameter(ps).impact());
	}
printf("runtime --- get states:\t%.1f ms\n", (getCurrentTime()-sysTime).value());
sysTime = getCurrentTime();


	model          = tmpModel;
	discountRate   = repatd.discountrate();
	traceDecayRate = repatd.tracedecayrate();
	learningRate   = repatd.learningrate();
	traceLength    = repatd.tracelength();

	granularityRatios   = computeGranularityRatios();
	radialExpansionList = computeRadialExpansion();

printf("runtime --- setup:\t%.1f ms\n", (getCurrentTime()-sysTime).value());
sysTime = getCurrentTime();

	eligibilityTrace.clear();
	Q.init(model);

	if (repatd.q().has_data()) {
		std::istringstream data( repatd.q().data() );

		unsigned int stateIndex = 0;
		float value = 0.;

		do {
			char learned;
			data.read( &learned, 1 );

			// check for EOF while reading
			if (data.eof())
				break;

			if (learned != '0' && learned != '1') {
				ERROR("Read invalid value.");
				break;
			}

			// if this state was learned, read the actions
			if (learned == '1') {
				for (unsigned int actionIndex = 0; actionIndex < Q.getNumberOfActionsPerState(); actionIndex++) {
					data.read( reinterpret_cast< char* >( &value ), sizeof(value) );
					Q[stateIndex].setAction(actionIndex, value);
				}
			}

			// next state
			stateIndex++;
		} while (false == data.eof());
printf("Read %d states\n", stateIndex);

	} else if (repatd.q().entry_size() > 0) {
		for (int e = 0; e < repatd.q().entry().size(); e++) {
			int s_idx   = repatd.q().entry(e).state();
			int a_idx   = repatd.q().entry(e).action();
			float value = repatd.q().entry(e).value();
			Q[s_idx].setAction(a_idx, value);
		}
	}

printf("runtime --- read entries:\t%.1f ms\n", (getCurrentTime()-sysTime).value());
sysTime = getCurrentTime();


	printf("REPATD: LOADING DONE.\n");

	isLoading.store(false);
	loadingSucceeded = true;

	return true;
}



/*------------------------------------------------------------------------------------------------*/



void RL_REPATD::worker() {
	while (   false == isRunning.load()
	       || false == initialized.load()
	       || true == isSaving.load()
	       || true == isLoading.load())
	{
		std::this_thread::yield();
	}


	printf("REPATD: Worker Thread started\n");

	while (isRunning.load()) {
		workerAtWork = true;
		RadialExpansionJob job;
		bool thereIsNewJob = jobQueue.dequeue(job);

		if (thereIsNewJob) {
			performRadialExpansion(job.stateIndex, job.actionIndex, job.targetQ, job.traceDiscount);
		}
		workerAtWork = false;
	}
	printf("REPATD: Worker Thread DONE.\n");
}



