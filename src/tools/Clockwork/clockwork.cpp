/*
 * clockwork.cpp
 *
 *  Created on: Mar 14, 2014
 *      Author: stepuh
 */

#include "clockwork.h"
#include "stdio.h"

Clockwork::Clockwork() {
	// TODO Auto-generated constructor stub
}


Clockwork::~Clockwork() {
	// TODO Auto-generated destructor stub
}


Clockwork::Clockwork(std::vector<int> _minValues, std::vector<int> _maxValues)
	: n_dim(_minValues.size())
	, minValues(_minValues)
	, maxValues(_maxValues)
	, currValues(_minValues)
{
}


std::vector<int> Clockwork::getValue() const {
	return this->currValues;
}


void Clockwork::setToValue(const std::vector<int> &values) {
	this->currValues = values;
}


void Clockwork::setToMinValue() {
	this->currValues = this->minValues;
}


void Clockwork::setToMaxValue() {
	this->currValues = this->maxValues;
}


void Clockwork::setMinValues(const std::vector<int> &newMinValues) {
	this->minValues = newMinValues;
}


void Clockwork::setMaxValues(const std::vector<int> &newMaxValues) {
	this->maxValues = newMaxValues;
}



void Clockwork::increment() {
	this->currValues[n_dim-1]++;

	// propagate overhead over all dimensions if necessary
	for (int n = this->n_dim-1; n >= 0; n--) {

		// is there overhead?
		if (this->currValues[n] > this->maxValues[n]) {

			this->currValues[n] = this->minValues[n];

			// not the very first number
			if (n-1 >= 0) {
				this->currValues[n-1]++;
			}


		// if there is no overhead in this dimension
		// then there is also none in any other dimension
		} else {
			return;
		}
	}
}


void Clockwork::incrementBy(unsigned int n) {
	for (unsigned int i = 0; i < n; i++) {
		this->increment();
	}
}


void Clockwork::decrement() {
	this->currValues[n_dim-1]--;

	// propagate overhead over all dimensions if necessary
	for (int n = this->n_dim-1; n >= 0; n--) {

		// is there overhead?
		if (this->currValues[n] < this->minValues[n]) {

			this->currValues[n] = this->maxValues[n];

			// not the very first number
			if (n-1 >= 0) {
				this->currValues[n-1]--;
			}


		// if there is no overhead in this dimension
		// then there is also none in any other dimension
		} else {
			return;
		}
	}
}


void Clockwork::decrementBy(unsigned int n) {
	for (unsigned int i = 0; i < n; i++) {
		this->increment();
	}
}


int Clockwork::getPossibleCombinations() const {
	int possibleCombinations = 1;

	for (int n = this->n_dim-1; n >= 0; n--) {
		int range = this->maxValues[n] - this->minValues[n] + 1;
		possibleCombinations *= range;
	}

	return possibleCombinations;
}


bool Clockwork::isGreater(Clockwork otherClockwork) const {
	if (this->n_dim > otherClockwork.n_dim) {
		return true;

	} else if (this->n_dim < otherClockwork.n_dim) {
		return false;

	} else {
		for (int n = 0; n < this->n_dim; n++) {
			if (this->currValues[n] < otherClockwork.currValues[n]) {
				return false;
			}
		}

		// could still be equal
		if (this->currValues[this->n_dim-1] == otherClockwork.currValues[this->n_dim-1]) {
			return false;
		} else {
			return true;
		}
	}
}


bool Clockwork::isSmaller(Clockwork otherClockwork) const {
	if (this->n_dim < otherClockwork.n_dim) {
		return true;

	} else if (this->n_dim > otherClockwork.n_dim) {
		return false;

	} else {
		for (int n = 0; n < this->n_dim; n++) {
			if (this->currValues[n] > otherClockwork.currValues[n]) {
				return false;
			}
		}

		// could still be equal
		if (this->currValues[this->n_dim-1] == otherClockwork.currValues[this->n_dim-1]) {
			return false;
		} else {
			return true;
		}
	}
}


bool Clockwork::isEqualTo(Clockwork otherClockwork) const {
	if (this->n_dim == otherClockwork.n_dim) {
		for (int n = 0; n < this->n_dim; n++) {
			if (this->currValues[n] != otherClockwork.currValues[n]) {
				return false;
			}
		}
		return true;

	} else {
		return false;
	}
}


void Clockwork::print() const {
	printf("Clockwork :: [");
	for (int i = 0; i < n_dim; i++) {
		printf("  %d  ", this->currValues[i]);
	}
	printf("]\n");
}


