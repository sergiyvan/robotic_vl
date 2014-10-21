/*
 * randomTreeNode.h
 *
 *  Created on: 14.03.2014
 *      Author: lutz
 */

#ifndef RANDOMTREENODE_H_
#define RANDOMTREENODE_H_

#include <vector>
#include <armadillo>

class RandomTreeNode {
public:

	RandomTreeNode(arma::colvec value, RandomTreeNode *parent = NULL)
		: mValue(value)
		, mChildren()
		, mParent(parent)
		, merror(0.)
	{
	}

	virtual ~RandomTreeNode() {
		for (RandomTreeNode *child : mChildren)
		{
			delete child;
		}
	}

	void addChild(RandomTreeNode *child) {
		mChildren.push_back(child);
	}

	std::vector<RandomTreeNode*> &getChildren() {
		return mChildren;
	}

	const std::vector<RandomTreeNode*> &getChildren() const {
		return mChildren;
	}

	RandomTreeNode * getParent() const {
		return mParent;
	}

	void setParent(RandomTreeNode *newParent) {
		mParent = newParent;
	}

	inline const arma::colvec &getValue() const {
		return mValue;
	}

	inline void setValue(arma::colvec newValue) {
		mValue = newValue;
	}

	inline void setError(double newError) {
		merror = newError;
	}

	inline double getError() const {
		return merror;
	}

private:
	arma::colvec mValue;
	std::vector<RandomTreeNode*> mChildren;
	RandomTreeNode *mParent;
	double merror;

};

#endif /* RANDOMTREENODE_H_ */
