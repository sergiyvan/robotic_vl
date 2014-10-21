#ifndef SERVICES_H_
#define SERVICES_H_

#include "servicesBase.h"
#include "utils/patterns/singleton.h"


/*------------------------------------------------------------------------------------------------*/

// forward declarations for services, to avoid having to
// make this a huge include-fest
class RobotModel;

// events we provide
DECLARE_EVENT_ID(EVT_COGNITION_MODULES_READY);
DECLARE_EVENT_ID(EVT_MOTION_MODULES_READY);


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
	virtual bool init(int argc, char* argv[], ServiceInitStructure* sis = nullptr) override;

	inline RobotModel&      getRobotModel()      { return *robotModel; }

protected:
	RobotModel      *robotModel;      /// representation of the robot model
};

extern Services &services;

#endif /* SERVICES_H_ */
