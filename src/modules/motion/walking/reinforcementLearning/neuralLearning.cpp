

#include "neuralLearning.h"

#include <fstream>
#include <iostream>
#include <thread>

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "utils/utils.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>


using namespace std;




/*------------------------------------------------------------------------------------------------*/


NeuralLearning::NeuralLearning()
	: model(NULL)
	, discountRate(0.0)
	, learningRate(0.0)
	, traceLength(0)
	, traceDecayRate(0)
	, isRunning(false)
	, jobQueueLength(0)
{
	// Empty!
}



/*------------------------------------------------------------------------------------------------*/



NeuralLearning::~NeuralLearning() {
}



/*------------------------------------------------------------------------------------------------*/



void NeuralLearning::init(RL_Model* _model, float _discountRate, float _traceDecayRate, float _learningRate, int _traceLength) {
	model              = _model;
	discountRate       = _discountRate;
	traceDecayRate     = _traceDecayRate;
	learningRate       = _learningRate;
	traceLength        = _traceLength;
	//isSaving = false;

	isRunning = true;

	//jobQueue = new ThreadSafeQueue<NeuralJob>(1000);


	printf("Creating neural networks...\n");
	Q_ANNs.clear();
	for (unsigned int i = 0; i < model->numberOfActions; i++) {
		SL_NeuronalNetwork* network = new SL_NeuronalNetwork();
		int n_inputs = model->getDefaultState().parameters.size();
		int n_hidden = n_inputs;
		network -> init(std::vector<int>({n_inputs, n_hidden, 1}));
		Q_ANNs.push_back(network);
	}


	printf("Starting worker thread for NeuralLearning...\n");
	cs.setName("NEURAL LEARNING WORKER");
	std::thread t(&NeuralLearning::worker, this);
	t.detach();
}



/*------------------------------------------------------------------------------------------------*/



// Compute: max_a : Q(s, a)
float NeuralLearning::getBestQValueForState(const RL_State& s) const {
	float bestValue = -9999;

	// Try out each possible action
	for (auto it(model->actions.begin()); it != model->actions.end(); ++it) {
		float tmpValue = getQValue(s, it->second);

		if (tmpValue > bestValue) {
			bestValue = tmpValue;
		}
	}

	return bestValue;
}



/*------------------------------------------------------------------------------------------------*/



// Compute: argmax_a : Q(s, a)
RL_Action NeuralLearning::getBestActionForState(const RL_State& s) const {
	float bestValue = -9999;
	RL_Action bestAction = model->getRandomAction();

	for (auto it(model->actions.begin()); it != model->actions.end(); ++it) {
		float tmpValue = getQValue(s, it->second);
		if (tmpValue > bestValue) {
			bestValue = tmpValue;
			bestAction = it->second;
		}
	}

	// The best value is exactly -9999 because there is no xp yet.
	if (bestValue == -9999) return model->getRandomAction();

	return bestAction;
}



/*------------------------------------------------------------------------------------------------*/


RL_Action NeuralLearning::getEGreedyActionForState(const RL_State& s, float epsilon) const {
	epsilon *= 1000;
	float random = rand() % 1000;
	if (random > epsilon) {
		return getBestActionForState(s);
	} else {
		return model->getRandomAction();
	}
}


/*------------------------------------------------------------------------------------------------*/


void NeuralLearning::updateBySample(const RL_Sample& sample) {
	//if (isSaving == true) return; // Don't update if current model is being saved

	float targetQ = sample.reward + discountRate * getBestQValueForState(sample.succState);
	propagateThroughEligibilityTrace(sample.state, sample.action, targetQ);
}


/*------------------------------------------------------------------------------------------------*/

void NeuralLearning::propagateThroughEligibilityTrace(const RL_State& s, const RL_Action& a, float targetQ) {
	// at (s, a) the eligibility trace is set to 1
	updateQ(s, a, targetQ, 1);
	//jobQueue -> enqueue_force({s, a, targetQ, 1});
//	jobQueue.push({s, a, targetQ, 1});


	float traceDiscount = 1.0;
	for (auto it(eligibilityTrace.begin()); it != eligibilityTrace.end(); ++it) {
		std::pair<RL_State, RL_Action> pair = *it;
		RL_State tmp_s  = pair.first;
		RL_Action tmp_a = pair.second;

		// at every pair (s', a') where s' == s the eligibility trace is set to 0
		if (false == tmp_s.isEqualToState(s)) {
			updateQ(tmp_s, tmp_a, targetQ, traceDiscount);
//			jobQueue -> enqueue_force({tmp_s, tmp_a, targetQ, traceDiscount});
//			jobQueue.push({tmp_s, tmp_a, targetQ, traceDiscount});

			traceDiscount =  traceDiscount * traceDecayRate * discountRate;
		} else {
			// erase while iterating - this is a valid method to do this. if you don't believe me: look it up yourself!
			it = eligibilityTrace.erase(it);
		}
	}

	// Push this experience in eligibility trace
	eligibilityTrace.push_front(std::pair<RL_State, RL_Action>(s, a));

	// If eligibility trace got too long, we cut off the last part
	if (int(eligibilityTrace.size()) > traceLength) {
		eligibilityTrace.pop_back();
	}
}



/*------------------------------------------------------------------------------------------------*/



void NeuralLearning::updateQ(const RL_State& s, const RL_Action& a, float targetQ, float traceDiscount) {
	printf("updateQ::targetQ: %f\n", targetQ);
	//int a_idx = model->getIndexForAction2(a);
	int a_idx = model->getIndexForAction(a);


	//float qValue = getQValue(s, a);
	//float error = (targetQ - qValue);
	//float targetValue = qValue + traceDiscount * error;

	arma::vec pattern  = generatePattern(s, a);
	//arma::vec teaching = arma::ones(1) * targetValue;
	arma::vec teaching = arma::ones(1) * targetQ;


	Q_ANNs[a_idx] -> teachPattern(pattern, teaching, learningRate * traceDiscount);
}



/*------------------------------------------------------------------------------------------------*/



float NeuralLearning::getQValue(const RL_State& s, const RL_Action& a) const {
	//int a_idx = model->getIndexForAction2(a);
	int a_idx = model->getIndexForAction(a);
	arma::vec pattern = generatePattern(s, a);
	arma::vec output = Q_ANNs[a_idx] -> getOutputForPattern(pattern);
	return output(0);
}



/*------------------------------------------------------------------------------------------------*/



arma::vec NeuralLearning::generatePattern(const RL_State& s, const RL_Action& a) const {
	//arma::vec pattern = arma::ones(s.parameters.size() + a.parameters.size());
	arma::vec pattern = arma::ones(s.parameters.size());


	int index = 0;
	//for (std::pair<std::string, int> e : s.parameters) {
	for (auto it(s.parameters.begin()); it != s.parameters.end(); ++it) {
		pattern(index) = it -> second;
		//pattern(index) = double(it -> second - model->parameterSpaces[it->first].minValue) / double(model->parameterSpaces[it->first].range);
		index++;
	}

	/*
	for (auto it(a.parameters.begin()); it != a.parameters.end(); ++it) {
		pattern(index) = it -> second;
		//pattern(index) = double(it -> second + model->parameterSpaces[it->first].impact) / (double(model->parameterSpaces[it->first].impact) * 2);
		index++;
	}
	*/

	/*
	printf("Pattern: {");
	for (unsigned int i = 0; i < pattern.n_rows; i++) {
		printf("%f, ", pattern(i));
	}
	printf("}\n");
	*/

	return pattern;
}



/*------------------------------------------------------------------------------------------------*/



void NeuralLearning::worker() {
	while (isRunning) {
		NeuralJob job;
		if (false == jobQueue.empty()) {
			job = jobQueue.front();
			jobQueue.pop();

			CriticalSectionLock lock(cs);
			updateQ(job.state, job.action, job.targetQ, job.traceDiscount);

			if (jobQueue.size() < 1000) {
				jobQueue.push(job);
			}
		}
	}
}



/*------------------------------------------------------------------------------------------------*/



void NeuralLearning::saveWhatWasLearnedAsync(string fileName) {
	//std::thread t(&RL_REPATD::saveWhatWasLearned, this, fileName);
	//t.detach();
}



/*------------------------------------------------------------------------------------------------*/



void NeuralLearning::loadWhatWasLearnedAsync(string fileName) {
	//std::thread t(&RL_REPATD::loadWhatWasLearnedOrBackup, this, fileName);
	//t.detach();
}



/*------------------------------------------------------------------------------------------------*/



bool NeuralLearning::loadWhatWasLearnedOrBackup(string fileName) {
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



void NeuralLearning::saveWhatWasLearned(string fileName) {
	/*
	bool expectedCurrentValue = false;
	if (false == isSaving.compare_exchange_strong(expectedCurrentValue, true)) return; // Don't save if a saving routine is already running

	printf("\nNOW SAVING WHAT WAS LEARNED!\n");

	MSG_RL_QLearning qlearning;
	qlearning.set_discountrate(discountRate);
	qlearning.set_learningrate(learningRate);
	qlearning.set_tracedecayrate(traceDecayRate);
	qlearning.set_tracelength(traceLength);
	qlearning.set_maxradialdistance(maxRadialDistance);


	MSG_RL_QLearning::Model *tmpModel = qlearning.mutable_model();
	for (auto it(model->parameterSpaces.begin()); it != model->parameterSpaces.end(); ++it) {
		MSG_RL_QLearning::Model::ParameterSpace* tmpParameterSpace = tmpModel->add_parameterspace();
		tmpParameterSpace->set_name(it->first);
		tmpParameterSpace->set_minvalue(it->second.minValue);
		tmpParameterSpace->set_maxvalue(it->second.maxValue);
		tmpParameterSpace->set_defaultvalue(it->second.defaultValue);
		tmpParameterSpace->set_impact(it->second.impact);
		tmpParameterSpace->set_withactions(it->second.withActions);
	}

	MSG_RL_QLearning::Matrix *tmpQMatrix = qlearning.mutable_q();
	for (auto it_s(Q.begin()); it_s != Q.end(); ++it_s) {
		MSG_RL_QLearning::Matrix::Row *tmpRow = tmpQMatrix->add_row();

		tmpRow->set_state(it_s->first);
		for (auto it_a(it_s->second.begin()); it_a != it_s->second.end(); ++it_a) {
			MSG_RL_QLearning::Pair *tmpPair = tmpRow->add_entry();
			tmpPair->set_action(it_a->first);
			tmpPair->set_value(it_a->second);
		}
	}

	if (!qlearning.IsInitialized()) {
		printf("Could not create Protobuf Message for QLearning!\n");
	} else {
		printf("Successfully created Protobuf Message.\n");
	}
	// create backup file
	std::string backupFileName = fileName + ".bak"; // backup copy
	if (fileExists(fileName.c_str())) {
		rename(fileName.c_str(), backupFileName.c_str());
		sync();
	}

	std::ofstream file(fileName, std::ios::out | std::ios::binary);
	if (false == file.fail()) {
		qlearning.SerializeToOstream(&file);
		sync();
		printf("Successfully saved Protobuf-Message for QLearning!\n");
	} else {
		printf("Could not save Protobuf-Message for QLearning!\n");
	}

	printf("SAVING DONE\n");
	isSaving = false;
	*/
}



/*------------------------------------------------------------------------------------------------*/



bool NeuralLearning::loadWhatWasLearned(string fileName) {
	/*

	// don't do anything if a load process is already running
	bool expectedCurrentValue = false;
	if (false == isLoading.compare_exchange_strong(expectedCurrentValue, true))
		return false;

	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (file.fail()) {
		ERROR("Could not open Protobuf-Message for QLearning!");
		return false;
	}

	google::protobuf::io::IstreamInputStream inputStream(&file);
	google::protobuf::io::CodedInputStream codedIStream(&inputStream);
	codedIStream.SetTotalBytesLimit(512*1024*1024, -1);

	MSG_RL_QLearning qlearning;
	qlearning.ParseFromCodedStream(&codedIStream);
	if (!qlearning.IsInitialized()) {
		ERROR("Could not parse Protobuf-Message for QLearning!");
		isLoading = false;
		return false;
	}


	RL_Model* tmpModel = new RL_Model;
	for (int ps = 0; ps < qlearning.model().parameterspace().size(); ps++) {
		tmpModel->addParameterSpace(qlearning.model().parameterspace(ps).name(),
				                    qlearning.model().parameterspace(ps).defaultvalue(),
				                    qlearning.model().parameterspace(ps).minvalue(),
				                    qlearning.model().parameterspace(ps).maxvalue(),
				                    qlearning.model().parameterspace(ps).impact(),
				                    qlearning.model().parameterspace(ps).withactions());
	}


	//init(tmpModel, qlearning.discountrate(), qlearning.tracedecayrate(), qlearning.learningrate(), qlearning.tracelength());
	init(tmpModel, qlearning.discountrate(), qlearning.tracedecayrate(), qlearning.learningrate(), qlearning.tracelength(), qlearning.maxradialdistance());

	Q.clear();
	for (int s = 0; s < qlearning.q().row().size(); s++) {
		int key_s = qlearning.q().row(s).state();
		for (int a = 0; a < qlearning.q().row(s).entry().size(); a++) {
			int key_a = qlearning.q().row(s).entry(a).action();
			float value = qlearning.q().row(s).entry(a).value();
			Q[key_s][key_a] = value;
		}
	}

	INFO("Walker parameters loaded");

	isLoading = false;
	 */
	return true;

}



