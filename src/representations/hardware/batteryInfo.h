#ifndef BATTERYINFO_H_
#define BATTERYINFO_H_

#include "ModuleFramework/Serializer.h"

class BatteryInfo {
public:
	BatteryInfo() : batteryPercent(1.) {}

	double batteryPercent;

protected:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & batteryPercent;
	}
};

REGISTER_SERIALIZATION(BatteryInfo, 1)



#endif
