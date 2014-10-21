/*
 * randomTree.cpp
 *
 *  Created on: 14.03.2014
 *      Author: lutz
 */

#include "randomTree.h"
#include <assert.h>
#include <limits>
#include <utils/math/Math.h>

RandomTree::RandomTree()
	: mDimCnt(1)
	, mExplorationEpsilon(0.)
	, mGoalState(arma::zeros(1))
	, mStartState(arma::zeros(1))
	, mMaxDims(arma::zeros(1))
	, mMinDims(arma::zeros(1))
	, mDimWidths(arma::zeros(1))
	, mRoot(NULL)
	, mNodeCnt(0)
{

}
RandomTree::RandomTree(uint dimCnt, double maxStepWidth, double minDist, double explorationEpsilon, arma::colvec goalState, arma::colvec startState, arma::colvec minDims, arma::colvec maxDims)
	: mDimCnt(dimCnt)
	, mExplorationEpsilon(explorationEpsilon)
	, mGoalState(goalState)
	, mStartState(startState)
	, mMaxDims(maxDims)
	, mMinDims(minDims)
	, mNodeCnt(0)
{
	mRoot = new RandomTreeNode(mStartState, NULL);
	mDimWidths = mMaxDims - mMinDims;

	mDistanceFunction = [] (arma::colvec a, arma::colvec b) {
			const arma::colvec diff = a - b;
			return arma::norm(diff, 2);
		};

	mGradientOptimizationFunction = [&] (arma::colvec pos) {
		return arma::zeros(mDimCnt);
	};

	mStepWidthFunction = [] (arma::colvec a) {
		return 5.; /* pfff... default distance all over the tree */
	};

	for (uint i = 0; i < mDimCnt; ++i)
	{
		assert(mDimWidths(i) >= 0);
	}
}

RandomTree::~RandomTree() {
	delete mRoot;
}

RandomTreeNode *RandomTree::getRoot()
{
	return mRoot;
}

void RandomTree::generateNodeQualities()
{
	uint nodeCnt = 0;
	generateNodeQualitiesSub(mRoot, nodeCnt);
}

double RandomTree::generateNodeQualitiesSub(RandomTreeNode *node, uint &nodeCnt)
{
	++nodeCnt;
	double thisNodesError = mDistanceFunction(mGoalState, node->getValue());

	for (RandomTreeNode *child : node->getChildren())
	{
		double subTreeError = generateNodeQualitiesSub(child, nodeCnt);
		thisNodesError = MIN(thisNodesError, subTreeError);
	}

	std::sort(node->getChildren().begin(), node->getChildren().end(), [](RandomTreeNode *a, RandomTreeNode *b)
			{
				return a->getError() < b->getError();
			});

	node->setError(thisNodesError);
	return thisNodesError;
}



RandomTreeNode *RandomTree::explore(uint amount)
{
	RandomTreeNode *newNode = NULL;
	for (uint i = 0; i < amount; ++i)
	{
		/* spawn node at random position */
		arma::colvec newState = mGoalState;
		double randVal = arma::randu();
		if (randVal > mExplorationEpsilon)
		{
			newState = arma::randu(mDimCnt);
			for (uint i = 0; i < mDimCnt; ++i)
			{
				newState(i) = newState(i) * mDimWidths(i);
			}
			newState += mMinDims;
		}

		/* find the closest node to the new one and append the new node to this */
		double bestNodeDist = std::numeric_limits<double>::max();
		RandomTreeNode *closestNode = const_cast<RandomTreeNode *>(getClosestNodeToState(mRoot, newState, bestNodeDist));

		/* clip to max StepWidth */
		arma::colvec bestStateDiff = newState - closestNode->getValue();
		double normOfDiff = mDistanceFunction(newState, closestNode->getValue());
		const double maxStepWidth = mStepWidthFunction(closestNode->getValue());
		if (normOfDiff > maxStepWidth) {
			bestStateDiff *= maxStepWidth / normOfDiff;
		}

		bestStateDiff += closestNode->getValue();

		newNode = new RandomTreeNode(bestStateDiff, closestNode);
		closestNode->addChild(newNode);
	}

	return newNode;
}

void RandomTree::optimize(double stopAtDistance)
{
	mNodeCnt = 0;
	/* do not optimize the root */
	mNodeCnt += optimizeSub(mRoot, false, stopAtDistance);
}

uint RandomTree::optimizeSub(RandomTreeNode *node, bool moveNode, double stopAtDistance)
{
	/* apply the gradient to the node */
	uint subChildrenCnt = 1;
	const double distToTarget = mDistanceFunction(mGoalState, node->getValue());
	if (false == moveNode || stopAtDistance > distToTarget)
	{
		/* do nothing */
	} else {
		/* optimize this node */
		arma::colvec oldValue = node->getValue();
		arma::colvec gradient = mGradientOptimizationFunction(oldValue);
		arma::colvec newValue = oldValue + gradient;

		node->setValue(newValue);
	}

	if (NULL != node->getParent())
	{
		/* limit the length of newValue to mMaxStepWidth */
		arma::colvec valueDiff = node->getValue() - node->getParent()->getValue();
		double norm = mDistanceFunction(node->getValue(), node->getParent()->getValue());
		const double maxStepWidth = mStepWidthFunction(node->getParent()->getValue());
		if (norm > maxStepWidth) {
			valueDiff *= maxStepWidth / norm;
		}
		arma::colvec limitedValue = node->getParent()->getValue() + valueDiff;
		node->setValue(limitedValue);
	}

	/* if this node's grandchild is about as close to us as the child then add the grandchild to this node */
	std::vector<RandomTreeNode *> additionalChildren;
	std::vector<RandomTreeNode *>::iterator childIter, childIter2, grandChildIter;

	for (RandomTreeNode *child : node->getChildren())
	{
		grandChildIter = child->getChildren().begin();
		const double distToChild = mDistanceFunction(node->getValue(), child->getValue());
		while (grandChildIter != child->getChildren().end())
		{
			bool canStepAhead = true;
			RandomTreeNode *grandChild = *grandChildIter;
			const double distToGrandChild = mDistanceFunction(grandChild->getValue(), node->getValue());
			if (distToGrandChild < distToChild)
			{
				/* remember the grandchildren */
				additionalChildren.push_back(grandChild);
				grandChildIter = child->getChildren().erase(grandChildIter);
				canStepAhead = false;
			}

			if (canStepAhead)
			{
				++grandChildIter;
			}
		}
	}

	/* add the new children */
	for (childIter = additionalChildren.begin(); childIter != additionalChildren.end(); ++childIter)
	{
		(*childIter)->setParent(node);
		node->getChildren().push_back(*childIter);
	}


	/* test if the children are "too close" and if so merge them */
	std::vector<std::pair<RandomTreeNode *, RandomTreeNode *> > nodesToMerge;
	childIter = node->getChildren().begin();
	for (; childIter != node->getChildren().end(); ++childIter)
	{
		RandomTreeNode *child1 = *childIter;

		childIter2 = childIter + 1;
		while (childIter2 != node->getChildren().end())
		{
			bool canStepAhead = true;
			RandomTreeNode *child2 = *childIter2;
			double dist = mDistanceFunction(child1->getValue(), child2->getValue());
			const double minDist = mStepWidthFunction(node->getValue()) / 2.;
			if (dist < minDist)
			{
				/* merge them */
				nodesToMerge.push_back({child2, child1});

				childIter2 = node->getChildren().erase(childIter2);
				canStepAhead = false;
			}
			if (canStepAhead)
			{
				++childIter2;
			}
		}
	}

	/* add the new children */
	for (std::pair<RandomTreeNode *, RandomTreeNode *> &toMerge : nodesToMerge)
	{
		RandomTreeNode * toDrop = toMerge.first;
		RandomTreeNode * mergeChildrenTo = toMerge.second;

		for (RandomTreeNode* &newChild : toDrop->getChildren())
		{
			newChild->setParent(mergeChildrenTo);
			mergeChildrenTo->addChild(newChild);
		}

		toDrop->getChildren().clear();
		delete toDrop;
	}


	for (RandomTreeNode *child : node->getChildren())
	{
		subChildrenCnt += optimizeSub(child, true, stopAtDistance);
	}


	return subChildrenCnt;
}

RandomTreeNode *RandomTree::getClosestNodeToState(RandomTreeNode *node, arma::colvec state, double &bestDist)
{
	RandomTreeNode *bestNode = node;

	/* test the node itself */
	double bestNodeDist = mDistanceFunction(state, node->getValue());

	for (RandomTreeNode *child : node->getChildren())
	{
		/* test if the child contains a node within its subtree which fits better */
		double bestNodeDistSubtree = bestNodeDist;
		RandomTreeNode *bestNodeFromSubTree = getClosestNodeToState(child, state, bestNodeDistSubtree);
		if (bestNodeDistSubtree < bestNodeDist)
		{
			bestNodeDist = bestNodeDistSubtree;
			bestNode = bestNodeFromSubTree;
		}
	}

	bestDist = bestNodeDist;
	return bestNode;
}

void RandomTree::getClosestNodesToState(const RandomTreeNode *node, arma::colvec state, double tolerance, std::vector<const RandomTreeNode*> &bestNodes) const
{
	/* test the node itself */
	double pathDist = node->getError();

	if (pathDist <= tolerance)
	{
		const double nodeDist = mDistanceFunction(node->getValue(), state);
		if (nodeDist <= tolerance)
		{
			bestNodes.push_back(node);
		}
		for (const RandomTreeNode *child : node->getChildren())
		{
			/* test if the child contains a node within its subtree which fits better */
			getClosestNodesToState(child, state, tolerance, bestNodes);
		}
	}
}


std::vector<const RandomTreeNode *> RandomTree::getBestPathToTarget(double tolerance) const
{
	std::vector<const RandomTreeNode *> ret;
	std::vector<const RandomTreeNode *> closestNodes;
	getClosestNodesToState(mRoot, mGoalState, tolerance, closestNodes);
	const RandomTreeNode * bestStartingNode = mRoot;
	double costsForBestPath = std::numeric_limits<double>::lowest();

	if (closestNodes.size() > 0)
	{
		for (const RandomTreeNode *startingNode : closestNodes)
		{
			/* calculate the costs for this path */
			double thisPathsQuality = 0.;
			const RandomTreeNode *traverseNode(startingNode);

			/* find the node which is closest to the target and closest to root */
			double bestDist = traverseNode->getError();
			double distToParent = 0;
			while (traverseNode != mRoot)
			{
				bestDist += distToParent * 0.9;
				const double curDist = mDistanceFunction(traverseNode->getValue(), mGoalState);
				if (bestDist > curDist)
				{
					bestDist = curDist;
					startingNode = traverseNode;
				} else {
					 distToParent = mDistanceFunction(traverseNode->getValue(), traverseNode->getParent()->getValue());
				}


				/* traverse up */
				traverseNode = traverseNode->getParent();
			}

			traverseNode = startingNode;

			while (traverseNode != mRoot)
			{
				arma::colvec gradient = mGradientOptimizationFunction(traverseNode->getParent()->getValue());
				arma::colvec direction = traverseNode->getValue() - traverseNode->getParent()->getValue();

				thisPathsQuality += arma::dot(gradient, direction);

				/* traverse up */
				traverseNode = traverseNode->getParent();
			}

			if (thisPathsQuality > costsForBestPath)
			{
				costsForBestPath = thisPathsQuality;
				bestStartingNode = startingNode;
			}
		}
	} else {
		// TODO: return closest node ?
		// or return nothing?
	}

	ret.push_back(bestStartingNode);
	while (bestStartingNode != mRoot)
	{
		bestStartingNode = bestStartingNode->getParent();
		/* traverse up */
		ret.push_back(bestStartingNode);
	}

	std::reverse(ret.begin(), ret.end());

	return ret;
}

void RandomTree::drawTree(RandomTreeNode *root, DebuggingOption *_debug_option, ImageDebugger &imageDebugger) const
{
	CIRCLE(root->getValue()(0), root->getValue()(1), 5);

	for (RandomTreeNode *child : root->getChildren())
	{
		LINE(root->getValue()(0), root->getValue()(1),
				child->getValue()(0), child->getValue()(1));
		drawTree(child, _debug_option, imageDebugger);
	}
}


/**
 * set a new root of the tree.
 * the new root must be a node from the tree every node which is related to the parent of the new root will be dropped
 * @param newRoot
 */
void RandomTree::setRoot(RandomTreeNode *newRoot, RandomTreeNode *becomesParentOf)
{
	mRoot = newRoot;
	RandomTreeNode *iter = becomesParentOf;
	while (NULL != iter)
	{
		RandomTreeNode *tmp = iter->getParent();
		newRoot->addChild(iter);
		iter->setParent(newRoot);
		iter = tmp;
	}
}
