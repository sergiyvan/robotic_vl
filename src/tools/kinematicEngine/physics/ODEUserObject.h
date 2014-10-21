/*
 * ODEUserObject.h
 *
 *  Created on: 02.06.2014
 *      Author: lutz
 */

#ifndef ODEUSEROBJECT_H_
#define ODEUSEROBJECT_H_

class ODEUserObject {
public:
	ODEUserObject();
	virtual ~ODEUserObject();

	bool canCollide;
};

#endif /* ODEUSEROBJECT_H_ */
