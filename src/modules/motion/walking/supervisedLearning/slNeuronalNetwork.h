/*
 * slNeuronalNetwork.h
 *
 *  Created on: Mar 2, 2013
 *      Author: stepuh
 */

#ifndef SLNEURONALNETWORK_H_
#define SLNEURONALNETWORK_H_

#include <math.h>
#include <vector>
#include <armadillo>




/**
 * The neuronal network that is implemented in this class is a feed forward network.
 * It can be used to learn a complicated function by training the network incrementally
 * with samples. A sample consists of a pattern (input-std::vector) and a teaching-std::vector.
 * The only method you should use are: init(), teachPattern() and getOutputForPattern().
 */

class SL_NeuronalNetwork {
public:

	std::vector<int> layerDescription;
	std::vector<arma::mat> oldWeights;      // For Resilient Propagation and Backpropagation with momentum
	std::vector<arma::mat> gradientSpeeds;  // For Resilient Propagation
	std::vector<arma::mat> weights;
	std::vector<arma::vec> outputs;
	std::vector<arma::mat> derivatives;
	std::vector<arma::vec> layerErrors;
	arma::vec outputErrors;


	/**
	 * Empty constructor
	 */
	SL_NeuronalNetwork();


	/**
	 * Initializes the feed forward neuronal network
	 *
	 * @param _layerDescription Vector with number of neurons in each layer
	 */
	void init(std::vector<int> _layerDescription);


	/**
	 * Computes the output for the function estimated by the neuronal network.
	 * The weights of the network are not being updated.
	 *
	 * @param inputPattern Input-Vector
	 * @return Output-Vector
	 */
	arma::vec getOutputForPattern(arma::vec inputPattern);


	/**
	 * Performs the Backpropagation algorithm. First the output-error is being computed,
	 * then the weights of the network are being updated.
	 *
	 * @param inputPattern Input-Vector
	 * @param teachingPattern The correct answer to the input-std::vector
	 * @param learningRate Speed of the gradient descent
	 */
	void teachPattern(arma::vec inputPattern, arma::vec teachingPattern, double learningRate);


	/**
	 * Helper function for the Backpropagation algorithm.
	 * Computes and stores the outputs and derivatives for each unit.
	 *
	 * @param inputPattern Input-Vector
	 * @param teachingPattern The correct answer to the input std::vector
	 */
	void feedForwardStep(arma::vec inputPattern, arma::vec teachingPattern);


	/**
	 * Helper function for the Backpropagation algorithm.
	 * Computes the layer errors for the output computed in
	 * the feed forward step.
	 */
	void backPropagationStep();


	/**
	 * Helper function for the Backpropagation algorithm.
	 * Updates the weights according to the derivatives and errors computed in
	 * the feed forward step.
	 *
	 * @param learningRate Speed of the gradient descent
	 */
	void classicWeightUpdate(double learningRate);


	/**
	 * Helper function for the Backpropagation algorithm.
	 * Updates the weights according to the derivatives and errors computed in
	 * the feed forward step. Does not use the gradient itself but only it's sign.
	 * Converges faster in many cases.
	 *
	 * @param learningRate Speed of the gradient descent
	 */
	void resilientWeightUpdate(double negativeAcceleration, double positiveAcceleration);


	/**
	 *
	 * @param learningRate
	 * @param momentum
	 */
	void momentumWeightUpdate(double learningRate, double momentum);


	/**
	 * Computes the sigmoid function that is used as the activation
	 * in the neuronal network.
	 *
	 * @param x Input
	 * @return Result of the sigmoid function in range of [0, 1]
	 */
	double sigmoid(double x);


	/**
	 * Computes element wise the sigmoid function for a given std::vector.
	 *
	 * @param x Input std::vector
	 * @return Output std::vector. Each element is the result of the sigmoid function.
	 */
	arma::vec sigmoid(arma::vec x);


	/**
	 * Augments the given std::vector with 1.
	 * E.g.: Given x = (x1, x2, ..., xn) -> result = (x1, x2, ..., xn, 1)
	 *
	 * @param x Input std::vector
	 * @return Augmented input std::vector
	 */
	arma::vec augmentOne(arma::vec x);


	/**
	 * Returns the sign of a given number.
	 * This method will return -1, if x < 0; It returns 0 if x = 0; and 1 if x > 0.
	 *
	 * @param x Number
	 * @return Sign of a giben number.
	 */
	double sign(double x);


	int net(double x);

	arma::vec net(arma::vec x);

	arma::vec maxToOne(arma::vec x);

};

#endif /* SLNEURONALNETWORK_H_ */
