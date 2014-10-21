#ifdef BERLINUNITEDSTANDALONE

#ifndef SERVICES_H_
#define SERVICES_H_

#include "servicesBase.h"

#include "utils/patterns/singleton.h"


/*------------------------------------------------------------------------------------------------*/

/**
 ** The Services class is the one-point-stop-shop for all
 ** things service-related.
 ** Or in other words - put all the crap here that needs to be initialized once
 ** and be available more or less globally.
 */

class Services
	: public ServicesBase
	, public Singleton<Services>
{
	friend class Singleton<Services>;

private:
	Services();

public:
	virtual ~Services();
};

extern Services &services;

#endif /* SERVICES_H_ */

#endif
