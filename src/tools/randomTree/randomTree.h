/*
 * randomTree.h
 *
 *  Created on: 14.03.2014
 *      Author: lutz
 */

#ifndef RANDOMTREE_H_
#define RANDOMTREE_H_

#include "randomTreeNode.h"

#include <debugging/imageDebugger.h>

#include <vector>
#include <armadillo>
#include <functional>

class RandomTree {
public:

	/* how to optimize the nodes */
	typedef std::function<arma::colvec(arma::colvec)> gradientOptimizationFunction;

	/* whats the distance between two states */
	typedef std::function<double(arma::colvec, arma::colvec)> distanceFunction;

	/* the maximum distance allowed for the given state (how far can another state be from this state */
	typedef std::function<double(arma::colvec)> stepWidthFunction;

	RandomTree();

	RandomTree(uint dimCnt, double maxStepWidth, double minDist, double explorationEpsilon, arma::colvec goalState, arma::colvec startState, arma::colvec minDims, arma::colvec maxDims);

	virtual ~RandomTree();

	RandomTreeNode *getRoot();

	RandomTreeNode *explore(uint amount);

	void setGoal(arma::colvec newGoal) {
		mGoalState = newGoal;
	}

	inline const arma::colvec &getGoal() const {
		return mGoalState;
	}

	void optimize(double stopAtDistance);


	/**
	 * set a new root of the tree.
	 * the new root must be a node from the tree every node which is related to the parent of the new root will be dropped
	 * @param newRoot
	 */
	void setRoot(RandomTreeNode *newRoot, RandomTreeNode *becomesParentOf);

	void setGradientOptimizationFunction(gradientOptimizationFunction gradFnt) {
		mGradientOptimizationFunction = gradFnt;
	}

	void setDistanceFunction(distanceFunction distFnt) {
		mDistanceFunction = distFnt;
	}

	void setStepWidthFunction(stepWidthFunction stepWidthFun) {
		mStepWidthFunction = stepWidthFun;
	}

	void drawTree(RandomTreeNode *root, DebuggingOption *_debug_option, ImageDebugger &imageDebugger) const;

	std::vector<const RandomTreeNode *> getBestPathToTarget(double tolerance) const;

	inline distanceFunction getDistanceFunction() const {
		return mDistanceFunction;
	}

	inline gradientOptimizationFunction getGradientOptimizationFunction() const {
		return mGradientOptimizationFunction;
	}

	inline stepWidthFunction getStepWidthFunction() const {
		return mStepWidthFunction;
	}

	inline uint getNodeCnt() const {
		return mNodeCnt;
	}

	void generateNodeQualities();

	RandomTreeNode *getClosestNodeToState(RandomTreeNode *node, arma::colvec state, double &bestDist);

private:
	uint mDimCnt;
	double mExplorationEpsilon;
	arma::colvec mGoalState, mStartState;
	arma::colvec mMaxDims, mMinDims, mDimWidths;
	RandomTreeNode *mRoot;

	gradientOptimizationFunction mGradientOptimizationFunction;
	distanceFunction mDistanceFunction;
	stepWidthFunction mStepWidthFunction;

	uint mNodeCnt;


	void getClosestNodesToState(const RandomTreeNode *node, arma::colvec state, double tolerance, std::vector<const RandomTreeNode*> &bestNodes) const;

	uint optimizeSub(RandomTreeNode *node, bool moveNode = true, double stopAtDistance = 0.);

	double generateNodeQualitiesSub(RandomTreeNode *node, uint &nodeCnt);
};

#endif /* RANDOMTREE_H_ */
