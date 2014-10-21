/*! \page add_new_motion Tutorial: How to add a new Motion

- Copy "my_motion.motion2" file to src/platform/hardware/robot/staticMotions2013.

- Build FUmanoids so a corresponding "my_motion.cmotion" is generated in the
same directory.

- Add a new entry in src/representations/motion/motionType.h.

- Add "include" of the .cmotion file and a new entry to
  "src/platform/hardware/robot/robotModel2013.cpp".


To make the motion executable from within XABSL, do the following:

- In "src/modules/cognition/behaviorLayer/xabsl/Symbol/MotionSymbols.xabsl" add
  a new enum entry for your motion. Furthermore in
  "src/modules/cognition/behaviorLayer/xabsl/Symbol/MotionSymbols.cpp" add a new
  entry to map the cpp enum to the xabsl enum.

- In "src/modules/cognition/behaviorLayer/xabsl/Options/MotionControl/MotionControl.xabsl"
  in the big "if ... else if" statement add a new case for your motion. Also add a
  new state.

*/
