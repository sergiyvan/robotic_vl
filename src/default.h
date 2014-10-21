/** @file
 **
 ** All includes listed in here are precompiled to a PCH file that
 ** can be parsed significantly faster in subsequent runs.
 **
 ** To see whether the precompiled header has any influence, check
 ** the compile time. You can also have GCC output information on the
 ** header files used. I.e. when copying one of the g++ commands and
 ** adding -H, you will get an output of all considered include files.
 ** If the precompiled header is used, it should be shown with a '!'
 ** in front of it (probably as the first entry in the list).
 */

#ifndef DEFAULT_H_
#define DEFAULT_H_

#ifdef USE_PRECOMPILED_HEADER

// List of all include files that are common. Adding includes that are
// not used by a majority of the files is probably counter-productive.
// Beware that using a precompiled header also means that you will have
// to recompile all cpp-files that make use of it if ANY of the include
// files listed below (or include files included by them) has changed!
// Therefore it is best to add system include files and files that are
// not changing often or are used everywhere anyway.

#include "robot.h"
#include "services.h"

#include "communication/comm.h"
#include "communication/messageRegistry.h"
#include "communication/remoteConnection.h"

#include "management/config/config.h"
#include "management/config/configProtobuf.h"
#include "management/config/configRegistry.h"
#include "management/commandLine.h"

#include "platform/system/events.h"
#include "platform/system/thread.h"
#include "platform/system/timer.h"
#include "platform/system/transport/transport.h"
#include "utils/patterns/singleton.h"

#include "utils/ansiTools.h"
#include "utils/math/Math.h"
#include "utils/math/Common.h"
#include "tools/position.h"

// debugging framework
#include "debug.h"
#include "debugging/debugging.h"
#include "debugging/imageDebugger.h"

// module framework
#include "ModuleFramework/Module.h"
#include "ModuleFramework/ModuleCreator.h"
#include "ModuleFramework/Serializer.h"

// root of our two top module managers
#include "modules/cognition/cognition.h"
#include "modules/motion/motion.h"

// common representations
#include "platform/image/image.h"
#include "representations/color.h"

// common protobuf files
#include <msg_message.pb.h>
#include <msg_modulelog.pb.h>

#include <google/protobuf/message.h>

// tool headers
#include <armadillo>

// system headers (C)
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// system headers (C++)
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#endif

#endif /* DEFAULT_H_ */
