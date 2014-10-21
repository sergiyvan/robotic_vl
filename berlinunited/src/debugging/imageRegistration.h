#ifndef IMAGEREGISTRATION_H__
#define IMAGEREGISTRATION_H__


#include "platform/system/macroMagic.h"

#define IMAGESTREAMENABLEDOPTION(name)  STRINGIFY(streaming.name.enabled)
#define IMAGESTREAMINTERVALOPTION(name) STRINGIFY(streaming.name.interval)
#define IMAGESTREAMUDPOPTION(name)      STRINGIFY(streaming.name.udp)

#define REGISTER_IMAGE_OPTIONS(name) \
	namespace { \
		auto cfg_##name##Enabled  = ConfigRegistry::registerOption(IMAGESTREAMENABLEDOPTION(name),                false,              "Whether streaming is enabled for this image"); \
		auto cfg_##name##Interval = ConfigRegistry::registerOption<Millisecond>(IMAGESTREAMINTERVALOPTION(name),  1000*milliseconds,  "Interval in ms for streaming this image"); \
		auto cfg_##name##UDP      = ConfigRegistry::registerOption<bool>(IMAGESTREAMUDPOPTION(name),              false,              "Whether to stream by UDP (1) or via TCP (0, requires remote port/ip)"); \
	}

#define REGISTER_IMAGE(name) \
		static bool __attribute__ ((unused)) UNIQUEVAR = ::Debugging::getInstance().registerImage(STRINGIFY(name)); \
		REGISTER_IMAGE_OPTIONS(name)


#endif
