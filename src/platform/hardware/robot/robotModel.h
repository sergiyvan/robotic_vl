#ifndef ROBOTMODEL_H
#define ROBOTMODEL_H

#include "utils/patterns/factory.h"
#include "utils/units.h"

// MotorID constants // TODO: get rid of that
#include "platform/hardware/robot/motorIDs.h"


#include <memory>
#include <set>
#include <string>

// forward declaration of hardware interfaces
class Actuators;
class Beeper;
class Buttons;
class IMU;
class LEDSignals;
class Power;
class Motions;
class Clock;

class RobotDescription;


/*------------------------------------------------------------------------------------------------*/

/** macro to register a robot model at the factory */
#define REGISTER_ROBOTMODEL(name, className, description) \
	REGISTER_OBJECT(name, className, RobotModel, description)

/*------------------------------------------------------------------------------------------------*/

/**
 ** Class representing a specific robot model, responsible for initializing
 ** the hardware and setting up particular classes that differ between the
 ** models.
 */

class RobotModel {
public:
	RobotModel();
	virtual ~RobotModel();

	// model creator
	static RobotModel* createRobotModel();

	inline const RobotDescription* getRobotDescription() {
		return robotDescription.get();
	}

	const Actuators*  getActuators()  const { return actuators.get(); }
	      Actuators*  getActuators()        { return actuators.get(); }
	const Beeper*     getBeeper()     const { return beeper.get(); }
	      Beeper*     getBeeper()           { return beeper.get(); }
	const Buttons*    getButtons()    const { return buttons.get(); }
	      Buttons*    getButtons()          { return buttons.get(); }
	const IMU*        getIMU()        const { return imu.get(); }
	      IMU*        getIMU()              { return imu.get(); }
	const LEDSignals* getLEDSignals() const { return ledSignals.get(); }
	      LEDSignals* getLEDSignals()       { return ledSignals.get(); }
	const Motions*    getMotions()    const { return motions.get(); }
	      Motions*    getMotions()          { return motions.get(); }
	const Power*      getPower()      const { return power.get(); }
		  Power*      getPower()            { return power.get(); }
	const Clock*      getClock()      const { return clock.get(); }
	      Clock*      getClock()            { return clock.get(); }

	virtual bool init();

	bool isHardwareInitialized() {
		return hardwareIsInitialized;
	}
	virtual std::map<MotorID, Degree> getIdleValues() const;

protected:
	bool hardwareIsInitialized;

	std::unique_ptr<Actuators>  actuators;
	std::unique_ptr<Beeper>     beeper;
	std::unique_ptr<Buttons>    buttons;
	std::unique_ptr<IMU>        imu;
	std::unique_ptr<LEDSignals> ledSignals;
	std::unique_ptr<Motions>    motions;
	std::unique_ptr<Power>      power;
	std::unique_ptr<Clock>      clock;

	std::unique_ptr<RobotDescription> robotDescription;
};

#endif
