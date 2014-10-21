#include <gtest/gtest.h>
#include "platform/system/thread.h"


class TestThread: public ::testing::Test {
protected:
	virtual void SetUp() {

	}
};

/* ------------------------------------------------------------------------- */

class MyThread : public Thread {
public:
	MyThread() {}
	virtual ~MyThread() {}

	virtual const char* getName() const {
		return "TestThread";
	}

	virtual void threadMain() override {
		// run for 5 seconds
		int a = 0;
		while (isRunning() && a < 50) {
			delay(100*milliseconds);
			a++;
		}
	}
};


/* ------------------------------------------------------------------------- */

TEST_F(TestThread, ThreadCreationAndCancel) {
	MyThread testThread;
	EXPECT_FALSE(testThread.isRunning());

	// start thread
	testThread.run();
	EXPECT_TRUE(testThread.isRunning());

	// cancel thread and wait for it to be cancelled
	testThread.cancel(true);
	EXPECT_FALSE(testThread.isRunning());
}


/* ------------------------------------------------------------------------- */

TEST_F(TestThread, ThreadCreationAndWait) {
	MyThread testThread;
	testThread.run();

	// wait for thread to be cancelled (5 seconds)
	testThread.wait();
	EXPECT_FALSE(testThread.isRunning());
}



/* ------------------------------------------------------------------------- */

TEST_F(TestThread, ThreadCreationAndWaitWithTimeout) {
	MyThread testThread;
	testThread.run();

	// wait for thread to be cancelled (1 second timeout)
	testThread.wait(1*seconds);
	EXPECT_TRUE(testThread.isRunning());
}

