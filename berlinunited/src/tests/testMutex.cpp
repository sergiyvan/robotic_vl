#include <gtest/gtest.h>
#include "platform/system/thread.h"


class TestMutex: public ::testing::Test {
protected:
	virtual void SetUp() {
	}
};

//TEST_F(TestMutex, TestNonRecursiveMutex) {
//	Mutex mutex(false);
//	EXPECT_TRUE(mutex.trylock());
//	EXPECT_FALSE(mutex.trylock());
//
//	// try unlock
//	mutex.unlock();
//	EXPECT_TRUE(mutex.trylock());
//	EXPECT_FALSE(mutex.trylock());
//
//	// have to unlock at end
//	mutex.unlock();
//}


TEST_F(TestMutex, TestRecursiveMutex) {
	Mutex mutex;
	EXPECT_TRUE(mutex.trylock());
	EXPECT_TRUE(mutex.trylock());
	EXPECT_TRUE(mutex.trylock());

	// try unlock
	mutex.unlock();
	mutex.unlock();
	mutex.unlock();

}
