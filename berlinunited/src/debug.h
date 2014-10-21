/** @defgroup debug Debugging System
@{

The Debugging System allows to send all kinds of debug infos which
can be evaluated with FUremote.

\section Introduction
You can get rid of all the Debugging stuff by defining NO_DEBUG, i.e.
g++ ... -DNO_DEBUG ...

Before sending anything you have to register at least one debug option
which is a "<name> <type>" pair. The syntax for the names is like
foo.bar.baz which would create the following tree:

 \verbatim
    foo
     |
     \-- bar
          |
          \-- baz
 \endverbatim

Available types so far:
- DRAWING_CAMERA: draw on the camera image
- DRAWING_FIELD: draw on the field
- DRAWING_VGA: draw on a raw canvas (640 x 480)
- PLOTTER: 2D plot
- TABLE: table of key-values
- TEXT: simple text messages
- STOPWATCH: time measurements (e.g. runtimes of code)

Beware that you need to register in a CPP file, never in a header!

\subsection Caveats Important Caveats regarding Debug Macros

The follow examples all make use of DEBUG macros that are meant to give you easy
and quick access to all of the possibilities the framework provides. However there
is an important caveat to note: the option name must be static (i.e. must never
change for that particular line of code).

 \verbatim
   DEBUG_TEXT("my.debug.option", "hello, world"); // OKAY

   void mydebug(std::string optionname, std::string text) {
       DEBUG_TEXT(optionname, text);
   }
   mydebug("my.debug.option.1", "hello, world");
   mydebug("my.debug.option.2", "and goodbye");  // NOT OKAY
 \endverbatim

The reason for this limitation is an optimization that minimizes the lookup
of the debug option. If you are in a situation where you need to re-use a
codeline or -block for debugging output with different option names, please
take a look at the macros and reimplement it in your code. E.g.

 \verbatim
   void mydebug(std::string optionname, std::string text) {
     ::Debugging::getInstance().sendDebugText(optionname, text);
   }
   mydebug("my.debug.option.1", "hello, world");
   mydebug("my.debug.option.2", "and goodbye");  // OKAY
 \endverbatim

\subsection Example_TEXT Example for TEXT

\code
// register the TEXT at the top of a cpp file
REGISTER_DEBUG("motion.walker", TEXT, BASIC);

[...]

// call the debug during execution of the module
DEBUG_TEXT("motion.walker", "this is a text message from walker");
DEBUG_TEXT("motion.walker", "%s %d", "value: ", 1234);
\endcode

\subsection Example_TABLE Example for TABLE

Tables contain a list of key-value pairs.

\code
// register the TABLE at the top of a cpp file
REGISTER_DEBUG("localization.loc", TABLE, BASIC);

[...]

// call the debug during execution of the module
DEBUG_TABLE("localization.loc", "robot position.angle", bestPosition.getAngle());
DEBUG_TABLE("localization.loc", "robot position.x", bestPosition.getX());
DEBUG_TABLE("localization.loc", "robot position.y", bestPosition.getY());
DEBUG_TABLE("localization.loc", "robot position belief", bestPosition.getBelief());
\endcode

\subsection Example_PLOTTER
\code
// register the PLOTTER at the top of a cpp file
REGISTER_DEBUG("localization.plotter", PLOTTER, BASIC);

[...]

// call the debug during execution of the module
currentTime = getCurrentTime()
DEBUG_PLOTTER("localization.plotter", "bestParticle belief", currentTime, bestPosition.getBelief());
DEBUG_PLOTTER("localization.plotter", "avg belief", currentTime, particleFilter.avgBelief());
DEBUG_PLOTTER("localization.plotter", "last beliefs", currentTime, getLocalizationBelief());
\endcode

\subsection Example_STOPWATCH

Stopwatches are grouped by the debug option. Per registered debug option, you can store as
many different stopwatches as required, giving them a unique name. Measurements are collected
and min/max/mean are calculated automatically. Do note that no data is sent out unless you
explicitely call STOPWATCH_SEND, in order to minimize the amount of traffic.

\code
// include header
#include "debug/stopwatch.h"

// register the STOPWATCH at the top of a cpp file
REGISTER_DEBUG("modules.cognition.runtimes", STOPWATCH, BASIC);

[...]

// call the debug during execution of the module
STOPWATCH_START("modules.cognition.runtimes", "Routine1");
routine1();
STOPWATCH_STOP("modules.cognition.runtimes", "Routine1");

// occassionally send out the stopwatch information
STOPWATCH_SEND("modules.cognition.runtimes");
\endcode

Optimization is included to keep the overhead in start/stop minimal, provided that the option
and stopwatch name are static (i.e. do not change).

\subsection Example_DRAWING

Drawings are made on the robot, send via network to FUremote and displayed there.

For the DRAWING_* debug types you have to add to your module header-file
\code
#include "representations/imageStream.h"

[...]

BEGIN_DECLARE_MODULE(MyModule)
	[...]
	PROVIDE(ImageStream)
END_DECLARE_MODULE(MyModule)
\endcode
In the cpp-File of your module you define a drawing option as follows:
\code
DRAWDEBUG(option, {
  // drawing commands and other code here
});
\endcode
You can insert normal code as well, thats nice, if you wan't to iterate over multiple objects
and draw all of them in the same way.

The following draw commands are available:
\code
// sets the color of the following commands to the given RGB-value
SETCOLOR(red, green, blue);
// some handy color definitions
SETCOLORRED;
SETCOLORGREEN;
...
// draw the outline of a circle
CIRCLE(x, y, radius);
// draw the outline of a rectangle
RECTANGLE(x, y, width, height)
// draw a filled rectangle
RECTANGLEFILLED(x, y, width, height)
// draw a line from (x1, y1) to (x2, y2)
LINE(x1, y1, x2, y2)
// draw a directed arrow for (x, y) with the given direction (in degree!) and length
ARROW(x, y, direction, length)
// draw a tetragon
TETRAGON(x1, y1, x2, y2, x3, y3, x4, y4)
// draw a filled tetragon
TETRAGONFILLED(x1, y1, x2, y2, x3, y3, x4, y4)
// draw a barchart
BARCHART(x, y, width, height, value)
\endcode
For more info see imageStream.h.

An example:
\code
DRAWDEBUG("my.draw.option", {
	SETCOLOR(255, 0, 0)
	RECTANGLE(100, 200, 10, 10)
	for(int i = 0; i < 4; ++i) {
		SETCOLOR(0, 20 * i, 20 * i)
		ARROW(30, 40, i*90, 10*i)
	}
});
\endcode
*/


#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <time.h>
#include <assert.h>

#include "platform/system/macroMagic.h"

#include "debugging/debugging.h"


/*------------------------------------------------------------------------------------------------*/


#ifdef NO_DEBUG

#define ASSERT(cond) ((void) 0)

#define REGISTER_DEBUG(option, optiontype, flags, ...)

#define DEBUG_CLASS(option, optiontype, debuggable, mslooptime) ((void) 0)

#define DEBUG_TEXT(option, ...)         ((void) 0)

#define DEBUG_PLOTTER(option, series, y, x) ((void) 0)

#define DEBUG_TABLE(option, key, payload) ((void) 0)

#define STOPWATCH(option, name, code) { code }
#define STOPWATCH_START ((void) 0)
#define STOPWATCH_STOP  ((void) 0)

#else

#define ASSERT(cond) assert(cond)

#define REGISTER_DEBUG(option, optiontype, flags, ...) \
		static bool __attribute__ ((unused)) UNIQUE(__LINE__) = \
		::Debugging::getInstance().registerDebugOption(option, optiontype, flags, __FILE__, ## __VA_ARGS__)

#define DEBUG_CLASS(option, optiontype, debuggable, mslooptime) \
		static bool __attribute__ ((unused)) UNIQUE(__LINE__) = \
		::DebuggingManager::getInstance().registerDebuggable(option, optiontype, debuggable, mslooptime)

#define DEBUG_TEXT(option, format, ...) \
		{ \
			static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
			if (_debug_option && _debug_option->enabled) { ::Debugging::getInstance().sendDebugText(_debug_option, format, ##__VA_ARGS__); } \
		}

#define DEBUG_PLOTTER(option, series, x, y) \
		{ \
			static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
			if (_debug_option && _debug_option->enabled) { ::Debugging::getInstance().sendDebugPlot(_debug_option, series, x, y); } \
		}

#define DEBUG_TABLE(option, key, payload) \
		{ \
			static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
			if (_debug_option && _debug_option->enabled) { ::Debugging::getInstance().sendDebugTable(_debug_option, key, payload); } \
		}

#define STOPWATCH(option, name, code) \
	{ \
		static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
		static StopwatchItem& _debug_stopwatch_item_ = Stopwatch::getInstance().getStopwatchReference(option, name); \
		if (_debug_option && _debug_option->enabled) Stopwatch::getInstance().notifyStart(_debug_stopwatch_item_, option, name); \
		{ code } \
		if (_debug_option && _debug_option->enabled) Stopwatch::getInstance().notifyStop(_debug_stopwatch_item_, option, name); \
	}

#define STOPWATCH_START(option, name) \
	{ \
		static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
		static StopwatchItem& _debug_stopwatch_item_ = Stopwatch::getInstance().getStopwatchReference(option, name); \
		if (_debug_option && _debug_option->enabled) Stopwatch::getInstance().notifyStart(_debug_stopwatch_item_, option, name); \
	}

#define STOPWATCH_STOP(option, name) \
	{ \
		static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
		static StopwatchItem& _debug_stopwatch_item_ = Stopwatch::getInstance().getStopwatchReference(option, name); \
		if (_debug_option->enabled) Stopwatch::getInstance().notifyStop(_debug_stopwatch_item_, option, name); \
	}

#define STOPWATCH_SEND(option) \
	{ \
		static DebuggingOption *_debug_option = ::Debugging::getInstance().getDebugOption(option); \
		if (_debug_option->enabled) Stopwatch::getInstance().send(option); \
	}

#endif /* NO_DEBUG */


/*------------------------------------------------------------------------------------------------*/


/*
 * The next three macros use a special feature of GNU CPP:
 *
 *    "the `##' token paste operator has a special meaning
 *    when placed between a comma and a variable argument"
 *
 * http://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
 *
 */

/**
 ** Print an error message and send it to FUremote.
 **
 */
#define ERROR(format, ...) \
	{ \
		::Debugging::getInstance().printError(format, ##__VA_ARGS__); \
		DEBUG_TEXT("Error", format, ##__VA_ARGS__) \
	}

/**
 ** Print a warning message and send it to FUremote.
 **
 */
#define WARNING(format, ...) \
	{ \
		::Debugging::getInstance().printWarning(format, ##__VA_ARGS__); \
		DEBUG_TEXT("Warning", format, ##__VA_ARGS__) \
	}

/**
 ** Print an info message and sent it ot FUremote.
 **
 */
#define INFO(format, ...) \
	{ \
		::Debugging::getInstance().printInfo(format, ##__VA_ARGS__); \
		DEBUG_TEXT("Info", format, ##__VA_ARGS__) \
	}


/** @} */

#include "debugging/imageDebugger.h"

#endif /* _DEBUG_H_ */

