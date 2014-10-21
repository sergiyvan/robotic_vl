#ifndef STATUS_H_
#define STATUS_H_

#include "platform/system/thread.h"
#include "utils/patterns/singleton.h"
#include "messages/msg_message.pb.h"

class StatusOutput : public Thread, public Singleton<StatusOutput> {
public:
	virtual ~StatusOutput();

	virtual const char* getName() const override {
		return "StatusOutput";
	}

	bool init();

	void sendStatus(const de::fumanoids::message::Message &status) {
		CriticalSectionLock lock(cs);
		cachedRobotStatus.CopyFrom(status);
		hasRobotStatusInfo = true;
	}

protected:
	virtual void threadMain() override;

	CriticalSection cs;
	de::fumanoids::message::Message cachedRobotStatus;
	bool hasRobotStatusInfo;
	robottime_t lastStatusTimestamp;

private:
	StatusOutput();
	friend class Singleton<StatusOutput>;
};

#endif /* STATUS_H_ */
