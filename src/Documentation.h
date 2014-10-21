/**
@mainpage FUmanoid Project Description

@section Introduction

This document should give you an overview of the FUmanoid system. If this
documentation feels incomplete (which it is), please feel free to
extend it!

The FUmanoid program gets started in main.cpp with main(). This sets up
the general application (termination handling, command line parsing, ...)
and then calls the Robot class to initialize the system and start the
requested action.

To test new code, you can add the test code to test.cpp and use the 'test'
command line argument to start the test routine.

We follow roughly the Sense-Think-Act paradigm.

\dotfile doc/sense-think-act.dot ["Sense-Think-Act"]


@section Overview
The project is divided as follows:
\verbatim
berlinunited
└── src                    The used copy of the Berlin United framework.

src
├── messages               Protobuf definitions for the FUmanoid messages
├── modules                Modules for cognition and motion.
│   ├── cognition
│   └── motion
├── platform               Internal tools that keep the program running.
│   ├── hardware             Hardware related support
│   └── mitecom              Mixed Team communication code (generic)
├── representations        All representations used by the modules.
├── simulation             Stuff for the Simulator.
├── tests                  Unit tests.
└── tools                  Various kind of tools like generic algorithms, etc
\endverbatim


@section FUmanoidSystem Architecture of the FUmanoid system

@subsection ModuleFrameworkSection        Module Framework

Most of the functionality in FUmanoid is implemented with the ModuleFramework.
Modules do some computation (e.g. modeling the ball). In order to do this they
require and provide some data, so called Representations (e.g. the BallPercept
and the BallModel).

Read \link ModuleFramework \endlink to fully understand the framework.

@note There shouldn't be any reason to change anything in the framework.
For implemented modules see \link ModulesSection \endlink.


@subsection ManagementSection             Management
@note After reading this section you should be able to configure you modules
and also various parts of the robot.

While developing robots it is necessary to adjust quite a few properties of the system.
FUmanoid offers a variaty of ways to do so.

- Parameters can be adjusted with FUremote or the commandline or a commandline editor. See \link config \endlink for more infos.
- REGISTER_COMMAND
- REGISTER_SWITCH

TODO extend

See \link management \endlink for more.


@subsection DebuggingSection              Debugging
FUmonoid can send out various information during runtime: text, plots, tables of values, images.
This can be debugged with FUremote. See \link debug \endlink for how to use it.

FUremote:

You can run FUremote, a Java based cross-platform application, to view different debug
information that FUmanoid sends out, including images and graphs. The requirement for
using FUremote is that they are in the same subnet (so UDP broadcasts can be used).

If you run FUmanoid locally (e.g. with the simulator) together with FUremote, you need
to manually adjust the communication ports. Keep FUremote as it is, but set FUmanoid to
run on a different port while still connecting to FUremote's port (11011 by default).
You do this by using --comm.port=11012 (for example) and --comm.remote.port=11011.

Logging:

It is also possible to record a log file. Each representation gets serialized
during each frame.  This makes it possible to replay everything later on a frame
by frame basis.

Here is an example of how to record a log file
\verbatim
./FUmanoid --logcognition
\endverbatim
You can do this on the robot but also on your computer running the simulator.
In that case start SimStar:
\verbatim
cd simulator; ./build/release/SimStar
\endverbatim
and then start FUmanoid which you compiled for the PC
\verbatim
cd build; python SIMmanoid.py --logcognition
\endverbatim

Replay a log:

To replay a log you have to start FUmanoid with the following command line parameters:
- specify a logfile with
	\verbatim
	--logplayer.logfile
	\endverbatim
- specify a comma seperated list of the modules you want to execute every frame:
	\verbatim
	--logplayer.activemodule=MultiHypManager,PoseGenerator
	\endverbatim
- add the "logplayer" command parameter to start FUmanoid in the logplayer mode:
	\verbatim
	logplayer
	\endverbatim

A full call looks like this:
\verbatim
./pc/FUmanoid --logplayer.logfile log/cognition_2012_7_20_13_55_51.pbl --logplayer.activemodule=MultiHypManager,PoseGenerator logplayer
\endverbatim

Offline pictures:

It is also possible to start FUmanoid with saved images instead of a real camera.
\verbatim
cd build; ./pc/FUmanoid --camera.type offline --camera.device sample_image.pbi
\endverbatim

The reporsitory 'calibrationdata' contains many pbi images.


@subsection ModulesSection                Modules for cognition and motion.
TODO extend


@subsubsection Behavior

For the behavior we use the DSL XABSL (see www.xabsl.org):
\verbatim
The Extensible Agent Behavior Specification Language XABSL is a very simple
language to describe behaviors for autonomous agents based on hierarchical
finite state machines. XABSL was developed to design the behavior of soccer
robots. ...
\endverbatim

TODO extend me

For our XABSL documentation see doc/xabsl/index.html


@subsection RepresentationSection        Representations of the Module-Framework.


@subsection CommunicationSection          Communication over the network.
Read \link comm \endlink.

@subsection SimulationSection             Simulator
The code is responsible for connecting to the simulator. Therefore, FUmanoid asks in its
initial message for a set of virtual sensors. In the acknowlegdement package the simulation
passes important values such as port numbers etc. to FUmanoid.
Based on the chosen configuration the simulation will send data through the specified connections,
which is handled transparently in FUmanoid - a module is not aware of the data source.

How to use the FUmanoid simulation software Sim*?

First, you should run an instance of Sim*. After a succesful compilation
step you can start an instance by using ./bin/start.sh (in simulation folder!).
In ./build/ you find an executable python script called SIManoid.py, which needs
several parameters depending of the data you want to receive from your virtual robot.

You can chose between four different modes. It will depend on your current tasks, which is
best for you.
- Vision only: Virtual robot with a network camera sensor
- Ground Truth only: Receive GT data without sending any data to the simulation software,
  e.g. motion status packages
- Behaviour (based on Vision): FUmanoids sends motion commands and receives camera data
- Behaviour (based on Ground Truth): FUmanoids sends motion commands and receives GT data
  which is provided as idealistic world modelling represenations.

Example call for SIManoid:
  $  ./SIManoid.py -m vision

You can also add configuration options or start special FUmanoid variations (e.g. walker)
  $  ./SIManoid.py -m vision --option.name value --option2.name value walker

For general terms use ./SIManoid.ph --help to get your command settings right.





@section General

@subsection CoordinateSystems Coordinate Systems

The coordinate sytem is in cm.

\image html doc/coordinate_systems.jpg


@subsection CodingConventions

\verbinclude doc/coding_style.txt "Taken from doc/coding_style.txt"

Here is an example of a good git commit msg:
\verbinclude doc/git_commit_msg_example.txt "Taken from doc/git_commit_msg_example.txt"
*/

/*------------------------------------------------------------------------------------------------*/

/**
@defgroup management Management

TODO add descr.
 */

/*------------------------------------------------------------------------------------------------*/

/**
@defgroup platform Platform

TODO add descr.
 */

/*------------------------------------------------------------------------------------------------*/

/**
@defgroup ModuleFramework Module Framework

taken (and extended) from 'NaoTH Software Architecture for an Autonomous Agent' by
'Heinrich Mellmann, Yuan Xu, Thomas Krause, and Florian Holzhauer' because
we use the same.

\section overview Overview

Our module framework is based on a blackboard architecture. It is used to
organize the workflow of the cognitive/deliberative part of the program. The
framework consists of the following basic components:
- Representation: is a objects carrying data, has no complex functionality.
- BlackBoard: is a container (data base) storing representations as information units.
- Module: is a executable unit, has access to the blackboard (can read and write
  representations).
- ModuleManager: manages the execution of the modules.

A module may require a representation, in this case it has a read-only access
to it. A module provides a representation, if it has a writing access. In our
design we consider only sequential execution of the modules, thus the there is
no handling for concurrent access to the blackboard necessary, i.e., it can be
decided during the compilation time.

We formulate the following requirements on the design of the module framework:
- the modules have no (direct) dependencies between each other (allows to
  remove or add a module)
- modules exchange information using the blackboard
- the required and provided representations are a static properties of a module,
  i.e., the blackboard is accessed during the construction time of a module
- it is always clear which representations are accessed by a module (i.e., it can
  be observed during the runtime)
- it is always clear which modules have access to a representation (i.e., it can
  be observed during the runtime)
- the representations don’t have any dependencies required by the framework

In order to provide a simple user interface we use C++ macros to hide the
mechanics of the framework. The following example
illustrates the usage of the framework from the point of view of a developer. Here an example for the
implementation of a module DemoModule which requires the ImageDimensions and provides the (nonsense)
LocalizationPosition.
\include modules/cognition/DemoModule/DemoModule.h DemoModule.h


The module needs to get registered at the according module manager (in this
case the Cognition) and execute() needs to get implemented.  Here is the
example:
\include modules/cognition/DemoModule/DemoModule.cpp DemoModule.cpp

The module gets then executed in the frame of the whole program.


\subsection representations Representations and Serialization

A representation is a dumb data object (with minimal functionality) which has
no dependencies.  Representations don't have any friend classes!

We can serialize the entire blackboard in a ModuleFrame (also see the Logger).
Therefore for every representation there needs to exist a Serializer. We use
ProtoBuf as to serialize our representations.  That means that we put the
important data into a proto message.

Most representations are points/objects on the field. RectangleObject
(basically everything that gets recognized by the vision) has a function
RectangleObject::asProto() which returns a ObjectOnField.  Take a look at
messages/msg_position.proto.

It's really easy to serialize representations with it.  Here is an example of
how to serialize the ball:
\include representations/imageProcessing/ballPercept.h

How to implement a representation:
- create the representation (a simple class) with the data and functionality
- put it into src/representations/ (or the according subfolder)
- create a proto file with the name of the representation
- put it in src/messages/
- implement the template Serializer for the class using the new proto file in the *.cpp of the representation.

Template programming makes sure, that the representation gets bound to the right
Serializer.


\subsection more more

For more info about concrete modules take a look at \ref ModulesAndRepresentations "Modules and Representations".

*/

/*------------------------------------------------------------------------------------------------*/


/**
@defgroup ModulesAndRepresentations Modules and Representations

The \ref ModuleFramework is used to implement the real functionality of the robot.

Read it first to get an overview of how it works.

We have two Module chains running: the Cognition chain with about 20 fps (triggered by the
camera) and the Motion chain which has a higher frequency (about 80).

Here is our current module structure:
\dotfile doc/modules_cognition.dot ["Cognition Modules"]
\dotfile doc/modules_motion.dot ["Motion Modules"]

*/


/*------------------------------------------------------------------------------------------------*/

/**
@defgroup cognition Cognition
@ingroup ModulesAndRepresentations

Triggered by the camera, the cognition evaluates new information (for example from the camera)
and acts accordingly.

*/
