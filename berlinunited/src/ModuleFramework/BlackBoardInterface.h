/**
 * @file BlackBoardInterface.h
 *
 * @author <a href="mailto:mellmann@informatik.hu-berlin.de">Heinrich Mellmann</a>
 * Implementation of class BlackBoardInterface
 * Implements an interface to access the blackboard
 *
 */

#ifndef __BlackBoardInterface_h_
#define __BlackBoardInterface_h_

#include "BlackBoard.h"
#include <iostream>

class BlackBoardInterface {
private:
	/** */
	BlackBoard* theBlackBoard;

	/** indicates whether this object has its own blackboard*/
	bool blackBoardOwner;

protected:
	/** inherits the blackboard */
	BlackBoardInterface(BlackBoard* theBlackBoard)
		: theBlackBoard(theBlackBoard)
		, blackBoardOwner(false)
	{}

	/** create its own blackboard */
	BlackBoardInterface()
		: theBlackBoard(new BlackBoard())
		, blackBoardOwner(true)
	{
	}

	virtual ~BlackBoardInterface() {
		if (blackBoardOwner) {
			delete theBlackBoard;
		}
	}

	BlackBoard& getBlackBoard() {
		assert(theBlackBoard != NULL);
		return *theBlackBoard;
	}

	const BlackBoard& getBlackBoard() const {
		assert(theBlackBoard != NULL);
		return *theBlackBoard;
	}

	// we are using pointers, it does not make sense to copy this handler so prevent it
	BlackBoardInterface(const BlackBoardInterface &) = delete;
	BlackBoardInterface& operator=(const BlackBoardInterface &) = delete;
};

#endif //__BlackBoardInterface_h_
