#ifndef MOTIONTYPE_H__
#define MOTIONTYPE_H__


/**
 * @brief All motions the robot can execute are represented through this enum.
 */
typedef enum {
	  MOTION_NONE                                       /// default value
	, MOTION_REMOTE                                     /// remote transmitted motion
	, MOTION_STAND_IDLE                                 /// idle motion
	, MOTION_LOCOMOTION                                 /// move the robot (walk, drive, fly, swim, etc)
	, MOTION_KICK                                       /// kick (adjustable by parameters)
	, MOTION_STANDUP                                    /// meta standup-motion, no actual motion mapped
	, MOTION_STANDUP_FRONT                              /// standup from front
	, MOTION_STANDUP_BACK                               /// standup from back
	, MOTION_PREPARE_LEFT_STANDUP                       /// prepare standup when lying on left side (outcome unknown)
	, MOTION_PREPARE_RIGHT_STANDUP                      /// prepare standup when lying on right side (outcome unknown)
//	, MOTION_PREPARE_STANDUP_FRONT                      /// prepare standup motion from front
//	, MOTION_PREPARE_STANDUP_BACK                       /// prepare standup motion from back
	, MOTION_THROWIN_PICKUP                             /// pick up the ball
	, MOTION_THROWIN_THROW                              /// throw in the ball
	, MOTION_INDICATE_LEFT                              /// indicate left side
	, MOTION_INDICATE_RIGHT                             /// indicate right side
	, MOTION_WAVE_LEFT                                  /// wink with left arm
	, MOTION_WAVE_RIGHT                                 /// wink with right arm
	, MOTION_CHICKEN_DANCE                              /// chicken dance. just in case something is very funny
	, MOTION_HIP_DANCE                                  /// another dance
	, MOTION_CHEER                                      /// cheer dance
	, MOTION_GOALIE_DOWN_LEFT                           /// goalie save left
	, MOTION_GOALIE_DOWN_RIGHT                          /// goalie save right
	, MOTION_GOALIE_PREPARE_STAND_UP_FROM_BACK          /// goalie go to pose for stand up from back
	, MOTION_SEMI_DYNAMIC_SHOT_LEFT_CAL                 /// motion for configuring the gyro roll that is used for the semi dynamic shot
	, MOTION_SEMI_DYNAMIC_SHOT_RIGHT_CONF               /// motion for configuring the gyro roll that is used for the semi dynamic shot
	, MOTION_CAMERA_SELFCALIBRATION                     /// right arm movement needed for selfcalibration
} MotionType;

#endif
