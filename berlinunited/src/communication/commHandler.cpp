/*
 * commHandler.cpp
 *
 *  Created on: Nov 26, 2011
 *      Author: dseifert
 */

#include "commHandler.h"


/*------------------------------------------------------------------------------------------------*/

/** Constructor
 **
 ** @param manager   Comm handler manager
 */

CommHandler::CommHandler(CommHandlerManager *manager)
	: manager(manager)
{
}


/*------------------------------------------------------------------------------------------------*/

/** Destructor
 **
 */

CommHandler::~CommHandler() {
	// nothing to see here
}
