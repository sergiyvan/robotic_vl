/*
 * rlmodelfactory.h
 *
 *  Created on: Feb 7, 2013
 *      Author: stepuh
 */
#include "rlModel.h"


#ifndef RLMODELFACTORY_H_
#define RLMODELFACTORY_H_


/**
 * This factory provides different models that can be used in reinforcement learning.
 * A model consists of: (State Space, Action Space, Transition Probabilities, Rewards)
 *
 * Concerning Actions: (Default Action)
 * ------------------------------------
 * The first Action that is put into the actions-vector (index 0) will be the default action.
 * This is because the policy-vector is initialized with zeros. So if the agent has no experience yet,
 * it will perform the default action.
 * -> You can use this information to define a stable/promising action as the default action.
 *
 *
 * Two words on initial rewards:
 * ----------------------------
 * FIRST: (Exploration v.s. Exploitation)
 * When rewards are very low at the beginning, the agent is unlikely to explore, because it fears new
 * situations. Only by a certain chance the agent chooses not to go for the learned policy. Only then
 * it will find out, that it actually was a good idea to perform another action.
 * But if the initial rewards are high, the agent explores a lot, because it thinks the world is full of rewards.
 * Only that it later eventually finds out, that it is actually not.
 * -> You can use this information to let the agent explore more or less often at the beginning.
 *
 * SECOND: (Use of Expert Wisdom)
 * If the rewardExperienceCount (for a certain action in a certain state) is very low at the beginning,
 * the agent has no idea about the rewards. That is why a new experience has a greater impact on the belief of the rewards.
 * In fact, if the rewardExperienceCount is set to 0, the initial reward will be replaced with the first experienced reward.
 * But if the rewardExperienceCount is very high at the beginning, the agent thinks it is an expert in estimating the rewards.
 * New experiences will have only little impact on the belief.
 * -> You can use this information to put your own expert wisdom in the system, or to 'load' already experienced Knowledge.
 */

class RL_ModelFactory {
public:

	/**
	 * Creates a very simple model for reinforcement learning.
	 * It uses a 2 dimensional state space (time and posY),
	 * as well as a one dimensional action space (posY) with 3 different actions.
	 * All transitions are deterministic. Rewards are still unknown and set to random.
	 *
	 * @return A very simple model for reinforcement learning
	 */
	static RL_Model* createSimpleModel();
};



#endif /* RLMODELFACTORY_H_ */
