#include <gtest/gtest.h>
#include "debugging/debugging.h"

class TestDebugging : public ::testing::Test {
protected:
	virtual void SetUp() {
	}
};

TEST_F(TestDebugging, DebugRegisterTrue) {
	
	EXPECT_TRUE(Debugging::getInstance().registerDebugOption("gtest", TEXT, BASIC));

	
	
	
}
