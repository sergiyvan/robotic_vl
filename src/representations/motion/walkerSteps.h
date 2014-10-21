/*
 * walkerSteps.h
 *
 *  Created on: 17.10.2014
 *      Author: lutz
 */

#ifndef WALKERSTEPS_H_
#define WALKERSTEPS_H_

#include <armadillo>
#include <list>
#include "supportFoot.h"

class WalkerStep {
public:
	enum Foot { LEFT = 1, RIGHT = 2};

	WalkerStep(Foot _foot, arma::mat44 _translation) : foot(_foot), translation(_translation)
	{ }

	virtual ~WalkerStep()
	{ }

	/**
	 * rotation and translation from the perspective of the previous step
	 */
	Foot foot;
	arma::mat44 translation;

};

class WalkerSteps {
public:
	WalkerSteps();
	virtual ~WalkerSteps();

	void addStep(WalkerStep const& step) {
		steps.push_back(step);
	}

	void setSupportFoot(SupportFoot foot) {
		supportFoot = foot;
	}

	SupportFoot const& getSupportFoot() const {
		return supportFoot;
	}

	std::list<WalkerStep> const& getSteps() const {
		return steps;
	}

	std::list<WalkerStep> steps;

	SupportFoot supportFoot;
};


#endif /* WALKERSTEPS_H_ */
