/*
 * namedInstance.h
 *
 *  Created on: Jul 3, 2014
 *      Author: dseifert
 */

#ifndef NAMEDINSTANCE_H_
#define NAMEDINSTANCE_H_

#include <string>

class NamedInstance {
public:
	virtual const char* getName() const = 0;
};


#endif /* NAMEDINSTANCE_H_ */
