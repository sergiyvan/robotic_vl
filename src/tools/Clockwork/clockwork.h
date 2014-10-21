/*
 * clockwork.h
 *
 *  Created on: Mar 14, 2014
 *      Author: stepuh
 */

#ifndef CLOCKWORK_H_
#define CLOCKWORK_H_

#include <vector>




// German: Zaehlwerk - e.g. digital watch
class Clockwork {
public:

	Clockwork();
	Clockwork(std::vector<int> _minValues, std::vector<int> _maxValues);
	virtual ~Clockwork();

	std::vector<int> getValue() const;
	void setToValue(const std::vector<int> &values);
	void setToMinValue();
	void setToMaxValue();

	void setMinValues(const std::vector<int> &newMinValues);
	void setMaxValues(const std::vector<int> &newMaxValues);

	void increment();
	void incrementBy(unsigned int n);
	void decrement();
	void decrementBy(unsigned int n);

	int getPossibleCombinations() const;

	bool isGreater(Clockwork otherClockwork) const;
	bool isSmaller(Clockwork otherClockwork) const;
	bool isEqualTo(Clockwork otherClockwork) const;

	void print() const;


private:
	int n_dim;                         // number of dimensions
	std::vector<int> minValues;        // minimal values for each dimension
	std::vector<int> maxValues;        // maximal values for each dimension
	std::vector<int> currValues;       // current configuration of the clockwork


};

#endif /* CLOCKWORK_H_ */
