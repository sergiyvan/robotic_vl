#include "management/config/configProtobuf.h"
#include "management/config/configTextFile.h"
#include "management/config/config.h"

#include <gtest/gtest.h>


/*------------------------------------------------------------------------------------------------*/

#define TEST_CONF_FILE "/tmp/dummy-configuration.conf"


/*------------------------------------------------------------------------------------------------*/

template <typename ConfigType>
class TestConfiguration : public ::testing::Test {
protected:
	Config *storage;
	int bla;

	TestConfiguration()
		: storage(nullptr)
		, bla(0)
	{}

	virtual void SetUp() {
		storage = new ConfigType();
		assert(storage);

		storage->registerOption<int>("nonempty", 0, "String Test Option");
		storage->set("nonempty", 7);
		bool success = this->storage->save(TEST_CONF_FILE);
		EXPECT_TRUE(success);
		delete storage;
		storage = nullptr;

		storage = new ConfigType();
		assert(storage);

		storage->registerOption<int>("a.b.c.i", 0, "Integer Test Option");
		storage->registerOption<float>("a.b.c.f", 0., "Float Test Option");
		storage->registerOption<std::string>("a.b.c.s", "", "String Test Option");

		success = storage->load(TEST_CONF_FILE);
		EXPECT_TRUE(success);
	}
};

typedef ::testing::Types<ConfigProtobuf, ConfigTextFile> ConfigTypes;
TYPED_TEST_CASE(TestConfiguration, ConfigTypes);


/*------------------------------------------------------------------------------------------------*/

TYPED_TEST(TestConfiguration, CreateOption) {
	Config* s = this->storage;

	bool success = false;

	success = s->exists("a.b.c.test");
	EXPECT_FALSE(success);

	s->registerOption<int>("a.b.c.test", 0, "Test Option");
	success = s->exists("a.b.c.test");
	EXPECT_TRUE(success);
}


TYPED_TEST(TestConfiguration, ChangeIntegerOptionValue) {
	Config* s = this->storage;

	bool success = false;
	int  value   = -1;

	success = s->exists("a.b.c.i");
	assert(success);

	s->getOption<int>("a.b.c.i")->set(42);

	value = s->get<int>("a.b.c.i");
	EXPECT_EQ(value, 42);
}


TYPED_TEST(TestConfiguration, ChangeStringOptionValue) {
	Config* s = this->storage;

	bool success = false;
	std::string strValue;

	success = s->exists("a.b.c.s");
	assert(success);

	s->getOption<std::string>("a.b.c.s")->set("42");

	strValue = s->get<std::string>("a.b.c.s");
	EXPECT_EQ(strValue, "42");
}


TYPED_TEST(TestConfiguration, ChangeFloatOptionValue) {
	Config* s = this->storage;

	bool success = false;
	float value   = -1;

	success = s->exists("a.b.c.f");
	assert(success);

	s->getOption<float>("a.b.c.f")->set(42.5);

	value = s->get<float>("a.b.c.f");
	EXPECT_EQ(value, 42.5);
}


TYPED_TEST(TestConfiguration, DefaultValues) {
	Config* s = this->storage;

	int  value   = 0;

	s->registerOption<int>("a.b.c.test", 0, "Test Option");

	// test default value
	value = s->get<int>("a.b.c.test");
	EXPECT_EQ(0, value);

	// test new default value
	value = s->get<int>("a.b.c.test", -1);
	EXPECT_EQ(-1, value);

	// test actual value
	s->getOption<int>("a.b.c.test")->set(1);
	value = s->get<int>("a.b.c.test", (int64_t)-1);
	EXPECT_EQ(1, value);
}

TYPED_TEST(TestConfiguration, SetFromProtobuf) {
	Config* s = this->storage;

	// register option with default value
	const int defaultValue = 42;
	auto cfg = s->registerOption<int>("a.b.c.protobuf", defaultValue, "Test Option");
	EXPECT_EQ(defaultValue, cfg->get());

	// set a protobuf option message with a set value (used == true)
	de::fumanoids::message::ConfigurationOption pbOption;
	pbOption.set_key("a.b.c.protobuf");
	pbOption.set_description("bla");
	pbOption.set_valid(true);
	pbOption.set_used(true);
	pbOption.set_type(de::fumanoids::message::ConfigurationOption_ValueType_INTEGER);
	pbOption.mutable_value()->set_value_int(defaultValue+1);
	pbOption.mutable_defaultvalue()->set_value_int(defaultValue+2);
	EXPECT_TRUE(pbOption.IsInitialized());

	cfg->fromProtobuf(&pbOption);
	EXPECT_EQ(defaultValue+1, cfg->get());
}

TYPED_TEST(TestConfiguration, UnsetValue) {
	Config* s = this->storage;

	// register option with default value
	const int defaultValue = 42;
	auto cfg = s->registerOption<int>("a.b.c.unset", defaultValue, "Test Option");
	EXPECT_EQ(defaultValue, cfg->get());

	// set a value
	cfg->set(defaultValue+1);
	EXPECT_EQ(defaultValue+1, cfg->get());

	// unset the value
	cfg->unset();
	EXPECT_EQ(defaultValue, cfg->get());
}

TYPED_TEST(TestConfiguration, UnsetValueFromProtobuf) {
	Config* s = this->storage;

	// register option with default value
	const int defaultValue = 42;
	auto cfg = s->registerOption<int>("a.b.c.unset", defaultValue, "Test Option");
	EXPECT_EQ(defaultValue, cfg->get());

	// set a value
	cfg->set(defaultValue+1);
	EXPECT_EQ(defaultValue+1, cfg->get());

	// set a protobuf option message with no set value (used == false)
	de::fumanoids::message::ConfigurationOption pbOption;
	pbOption.set_key("a.b.c.unset");
	pbOption.set_description("bla");
	pbOption.set_valid(true);
	pbOption.set_used(false);
	pbOption.set_type(de::fumanoids::message::ConfigurationOption_ValueType_INTEGER);
	pbOption.mutable_value(); // create empty value
	pbOption.mutable_defaultvalue()->set_value_int(defaultValue+2);
	EXPECT_TRUE(pbOption.IsInitialized());

	cfg->fromProtobuf(&pbOption);
	EXPECT_EQ(defaultValue, cfg->get());
}
