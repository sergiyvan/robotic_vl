#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include "platform/system/thread.h"
#include "utils/patterns/singleton.h"

class WatchDog : public Singleton<WatchDog>, public Thread {
public:
	virtual const char* getName() const override { return "WatchDog"; }
	friend class Singleton<WatchDog>;

protected:
	virtual void threadMain() override;

public:
	WatchDog();
	virtual ~WatchDog();

};


#endif /* WATCHDOG_H_ */
