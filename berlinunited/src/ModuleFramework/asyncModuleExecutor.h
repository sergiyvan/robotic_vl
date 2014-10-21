#ifndef ASYNCMODULEEXECUTOR_H_
#define ASYNCMODULEEXECUTOR_H_

#include "platform/system/thread.h"
#include "ModuleFramework/ModuleCreator.h"

#include "utils/units.h"

/** The AsyncModuleExecutor handles the execution of a module
 ** in a thread. This is helpful to detect hung modules.
 */

class AsyncModuleExecutor : public Thread {
public:
	AsyncModuleExecutor();
	virtual ~AsyncModuleExecutor();

	/// get a descriptive name of the thread
	virtual const char* getName() const override {
		return "AsyncModuleExecutor";
	}

	void executeModule(AbstractModuleCreator* module);
	bool waitForModuleToFinish(Millisecond timeout = 0*milliseconds);

protected:
	CriticalSection cs;
	ContinuousEvent moduleFinishedEvent;
	ContinuousEvent startModuleEvent;

	AbstractModuleCreator* module;

	/// thread main function, needs to be implemented by a thread
	virtual void threadMain() override;
};

#endif
