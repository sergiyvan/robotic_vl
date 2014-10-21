#include <gtest/gtest.h>
#include "platform/system/thread.h"


class TestEvent: public ::testing::Test {
protected:
	virtual void SetUp() {

	}
};


/* ------------------------------------------------------------------------- */

TEST_F(TestEvent, EventTriggerAndWait) {
	Event e;

	// trigger event asynchronously
	std::thread thread([&e]{
		delay(500*milliseconds);
		e.trigger();
	});

	// wait for event to be triggered
	bool success = e.wait(1000*milliseconds);
	EXPECT_TRUE(success);

	thread.join();
}

/* ------------------------------------------------------------------------- */

TEST_F(TestEvent, EventTriggerAndWaitAfterwards) {
	Event e;

	// trigger event, should awake all waiting threads (none in this case)
	e.trigger();

	// wait a bit for event to be triggered, should not happen
	bool success = e.wait(250*milliseconds);
	EXPECT_FALSE(success);
}

/* ------------------------------------------------------------------------- */

TEST_F(TestEvent, ContinuousEventTriggerAndWaitAfterwards) {
	ContinuousEvent e;

	// trigger event, should awake all waiting threads (none in this case)
	e.trigger();

	// wait a bit for event to be triggered, should happen
	bool success = e.wait(250*milliseconds);
	EXPECT_TRUE(success);
}
