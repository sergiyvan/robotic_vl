/*
 * slNeuronalNetwork.cpp
 *
 *  Created on: Mar 2, 2013
 *      Author: stepuh
 */

#include "slNeuronalNetwork.h"


using namespace std;


/*------------------------------------------------------------------------------------------------*/


SL_NeuronalNetwork::SL_NeuronalNetwork() {
	// Empty!
}


/*------------------------------------------------------------------------------------------------*/


void SL_NeuronalNetwork::init(vector<int> _layerDescription) {
	layerDescription = _layerDescription;

	// Reset vectors and matrices
	weights        = vector<arma::mat>(layerDescription.size() - 1, arma::mat(1, 1));
	oldWeights     = vector<arma::mat>(layerDescription.size() - 1, arma::mat(1, 1));
	gradientSpeeds = vector<arma::mat>(layerDescription.size() - 1, arma::mat(1, 1));
	outputs        = vector<arma::vec>(layerDescription.size(), arma::vec(1));
	derivatives    = vector<arma::mat>(layerDescription.size() - 1, arma::mat(1, 1));
	layerErrors    = vector<arma::vec>(layerDescription.size() - 1, arma::vec(1));

	// Initialize everything with 1
	for (unsigned int layer = 0; layer < layerDescription.size()-1; layer++) {
		// Weight-Matrix: [(m+1) x k], where m = #neurons in layer i; k = #neurons in layer i+1
		weights[layer] = arma::zeros(layerDescription[layer] + 1, layerDescription[layer+1]);
		//weights[layer] = arma::randu(layerDescription[layer] + 1, layerDescription[layer+1]);
		oldWeights[layer] = arma::zeros(layerDescription[layer] + 1, layerDescription[layer+1]);
		gradientSpeeds[layer] = arma::ones(layerDescription[layer] + 1, layerDescription[layer+1]) * 0.5;

		// Derivative-Matrix: [m x m], where m = #neurons in layer i+1;
		derivatives[layer] = arma::zeros(layerDescription[layer+1], layerDescription[layer+1]);

		// Layer-Error-Vector: [m], where m = #neurons in layer i+1;
		layerErrors[layer] = arma::ones(layerDescription[layer+1]);
	}

	for (unsigned int layer = 0; layer < layerDescription.size(); layer++) {
		// Output-Vector: [m], where m = #neurons in layer i;
		outputs[layer] = arma::ones(layerDescription[layer]);
	}

	outputErrors = arma::ones(layerDescription[layerDescription.size()-1]);
}


/*------------------------------------------------------------------------------------------------*/


arma::vec SL_NeuronalNetwork::getOutputForPattern(arma::vec inputPattern) {
	if (int(inputPattern.n_rows) != layerDescription[0]) {
		printf("outPutForPattern :: Wrong dimensions of input pattern! Given %d, expected: %d\n", int(inputPattern.n_rows), layerDescription[0]);
	}

	// Augment input vector with 1
	outputs[0] = inputPattern;
	inputPattern = augmentOne(inputPattern);

	// Propagate input through the layers
	arma::vec output;
	for (unsigned int layer = 1; layer < layerDescription.size(); layer++) {
		// Compute outputs for the current layer
		output = sigmoid(trans(arma::trans(inputPattern) * weights[layer-1]));

		// Augmented output of current layer is input of next layer
		inputPattern = augmentOne(output);
	}

	return output;
}

/*------------------------------------------------------------------------------------------------*/


void SL_NeuronalNetwork::teachPattern(arma::vec inputPattern, arma::vec teachingPattern, double learningRate) {
	if (int(inputPattern.n_rows) != layerDescription[0]) {
		printf("teachPattern :: Wrong dimensions of input pattern! Given %d, expected: %d\n", int(teachingPattern.n_rows), layerDescription[0]);
		return;
	}

	if (int(teachingPattern.n_rows) != layerDescription[layerDescription.size()-1]) {
		printf("teachPattern :: Wrong dimensions of teaching pattern! Given %d, expected: %d\n", int(teachingPattern.n_rows), layerDescription[layerDescription.size()-1]);
		return;
	}

	feedForwardStep(inputPattern, teachingPattern);
	backPropagationStep();
	classicWeightUpdate(learningRate);
	//resilientWeightUpdate(0.5, 1.2);
}


/*------------------------------------------------------------------------------------------------*/


void SL_NeuronalNetwork::feedForwardStep(arma::vec inputPattern, arma::vec teachingPattern) {
	// Augment input vector with 1
	outputs[0] = inputPattern;
	inputPattern = augmentOne(inputPattern);

	// Propagate input through the layers
	arma::vec output;
	for (unsigned int layer = 1; layer < layerDescription.size(); layer++) {
		// Compute outputs for the current layer
		output = sigmoid(trans(arma::trans(inputPattern) * weights[layer-1]));

		// Store outputs
		outputs[layer] = output;

		// Store derivatives in diagonal matrix
		// We do not update on input layer
		for (unsigned int unit = 0; unit < unsigned(layerDescription[layer]); unit++) {
			derivatives[layer-1](unit, unit) = output(unit) * (1.0 - output(unit));
		}

		// Augmented output of current layer is input of next layer
		inputPattern = augmentOne(output);
	}

	// Compute Output-Errors
	outputErrors = output - teachingPattern;
}


/*------------------------------------------------------------------------------------------------*/


void SL_NeuronalNetwork::backPropagationStep() {
	// Compute Layer-Errors
	layerErrors[layerDescription.size()-2] = derivatives[layerDescription.size()-2] * outputErrors;
	for (unsigned int layer = 0; layer < layerDescription.size()-2;  layer++) {
		int l = (layerDescription.size() - 2) - layer;
		arma::mat de_augmented_weights = weights[l];
		de_augmented_weights.resize(de_augmented_weights.n_rows-1, de_augmented_weights.n_cols);
		layerErrors[l-1] = derivatives[l-1] * de_augmented_weights * layerErrors[l];
	}
}


/*------------------------------------------------------------------------------------------------*/


void SL_NeuronalNetwork::classicWeightUpdate(double learningRate) {
	for (unsigned int layer = 0; layer < layerDescription.size()-1; layer++) {
		arma::mat weightDiff = - learningRate * (layerErrors[layer] * arma::trans(augmentOne(outputs[layer]))) ;
		weights[layer] = weights[layer] + arma::trans(weightDiff);
		//printf("weight diff\n");
		//weightDiff.print();
	}
}


/*------------------------------------------------------------------------------------------------*/


void SL_NeuronalNetwork::resilientWeightUpdate(double negativeAcceleration, double positiveAcceleration) {
	const double minSpeed = 0.000001;
	const double maxSpeed = 50.0;

	for (unsigned int layer = 0; layer < layerDescription.size()-1; layer++) {
		arma::mat partialDerivatives = arma::trans(layerErrors[layer] * arma::trans(augmentOne(outputs[layer])));

		for (unsigned int row = 0; row < weights[layer].n_rows; row++) {
			for (unsigned int col = 0; col < weights[layer].n_cols; col++) {
				//Update speeds
				if (partialDerivatives(row, col) * oldWeights[layer](row, col) > 0) {
					// gradient goes in same direction as before -> increase gradient speed
					gradientSpeeds[layer](row, col) = min(positiveAcceleration * gradientSpeeds[layer](row, col), maxSpeed);
					weights[layer](row, col) -= sign(partialDerivatives(row, col)) * gradientSpeeds[layer](row, col);
					oldWeights[layer](row, col) = partialDerivatives(row, col);

				} else if (partialDerivatives(row, col) * oldWeights[layer](row, col) < 0) {
					// gradient goes in other direction than before -> decrease gradient speed
					gradientSpeeds[layer](row, col) = max(negativeAcceleration * gradientSpeeds[layer](row, col), minSpeed);
					oldWeights[layer](row, col) = 0;

				} else {
					weights[layer](row, col) -= sign(partialDerivatives(row, col)) * gradientSpeeds[layer](row, col);
					oldWeights[layer](row, col) = partialDerivatives(row, col);
				}
			}
		}
	}
}

/*------------------------------------------------------------------------------------------------*/


void SL_NeuronalNetwork::momentumWeightUpdate(double learningRate, double momentum) {

}


/*------------------------------------------------------------------------------------------------*/


double SL_NeuronalNetwork::sigmoid(double x) {
	return 1.0 / (1.0 + exp(-x));
}


/*------------------------------------------------------------------------------------------------*/


arma::vec SL_NeuronalNetwork::sigmoid(arma::vec x) {
	for (unsigned int i = 0; i < x.n_elem; i++) {
		x(i) = sigmoid(x(i));
	}
	return x;
}


/*------------------------------------------------------------------------------------------------*/


arma::vec SL_NeuronalNetwork::augmentOne(arma::vec x) {
	x.resize(x.n_elem + 1);
	x(x.n_elem - 1) = 1.0;
	return x;
}


/*------------------------------------------------------------------------------------------------*/


double SL_NeuronalNetwork::sign(double x) {
	if (x > 0.0) {
		return 1.0;
	} else if (x < 0.0) {
		return -1.0;
	} else {
		return 0.0;
	}
}

/*------------------------------------------------------------------------------------------------*/


int SL_NeuronalNetwork::net(double x) {
	if (x < 0.5) {
		return 0;
	} else {
		return 1;
	}
}


/*------------------------------------------------------------------------------------------------*/


arma::vec SL_NeuronalNetwork::net(arma::vec x) {
	for (unsigned int i = 0; i < x.n_elem; i++) {
		x(i) = net(x(i));
	}
	return x;
}


/*------------------------------------------------------------------------------------------------*/


arma::vec SL_NeuronalNetwork::maxToOne(arma::vec x) {
	int maxIndex = 0;
	double maxValue = 0;
	for (unsigned int i = 0; i < x.n_elem; i++) {
		if (x(i) > maxValue) {
			maxValue = x(i);
			maxIndex = i;
		}
	}
	x = arma::zeros(x.n_elem);
	x(maxIndex) = 1;
	return x;
}



