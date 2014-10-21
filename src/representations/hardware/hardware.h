#ifndef HARDWARE_H
#define HARDWARE_H

#include "services.h"
#include "platform/hardware/robot/robotModel.h"

class Hardware {
public:
	Hardware() {
	}

	      Actuators*   getActuators()        { return services.getRobotModel().getActuators(); }
	const Actuators*   getActuators()  const { return services.getRobotModel().getActuators(); }
	      Beeper*      getBeeper()           { return services.getRobotModel().getBeeper(); }
	const Beeper*      getBeeper()     const { return services.getRobotModel().getBeeper(); }
	      Buttons*     getButtons()          { return services.getRobotModel().getButtons(); }
	const Buttons*     getButtons()    const { return services.getRobotModel().getButtons(); }
	      IMU*         getIMU()              { return services.getRobotModel().getIMU(); }
	const IMU*         getIMU()        const { return services.getRobotModel().getIMU(); }
	      LEDSignals*  getLEDSignals()       { return services.getRobotModel().getLEDSignals(); }
	const LEDSignals*  getLEDSignals() const { return services.getRobotModel().getLEDSignals(); }
		  Power*       getPower()            { return services.getRobotModel().getPower(); }
	const Power*       getPower()      const { return services.getRobotModel().getPower(); }
	      Clock*       getClock()            { return services.getRobotModel().getClock(); }
	const Clock*       getClock()      const { return services.getRobotModel().getClock(); }

};

#endif

