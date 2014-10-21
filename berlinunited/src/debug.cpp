#include "debug.h"

/* Register macros for standard messages */
REGISTER_DEBUG("Error", TEXT, ENABLED, "General error messages");
REGISTER_DEBUG("Warning", TEXT, ENABLED, "General warning messages");
REGISTER_DEBUG("Info", TEXT, ENABLED);
