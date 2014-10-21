#include "management/config/configRegistry.h"


namespace {
	auto cfg = ConfigRegistry::registerOption<bool>("seriallowlatency", true, "Whether to enable low latency in the Linux kernel serial driver");
}
