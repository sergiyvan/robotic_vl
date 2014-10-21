#ifndef MOTION_H
#define	MOTION_H

#include "ModuleFramework/ModuleManager.h"
#include "ModuleFramework/moduleManagers.h"

#include "management/config/configRegistry.h"
#include "platform/system/thread.h"
#include "services.h"

#include <fstream>


/*------------------------------------------------------------------------------------------------*/


/**
 * @addtogroup ModulesAndRepresentations
 *
 * @{
 */

/*------------------------------------------------------------------------------------------------*/

/**
 **
 */

class Motion
	: virtual public ModuleManager
	, virtual public Thread
{
public:
	Motion();
	virtual ~Motion();

	virtual const char* getName() const override {
		return "Motion";
	}

	virtual void startManager(int level) override;
	virtual void stopManager() override;

	virtual void threadMain() override;

private:
	CriticalSection startCS;
};

/**
 * @}
 */
#endif	/* MOTION_H */
