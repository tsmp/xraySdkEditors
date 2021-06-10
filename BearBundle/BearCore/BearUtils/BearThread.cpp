#include "BearCore.hpp"
#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#include "Windows/BearThread_Windows.h"
#elif CURRENT_PLATFORM == PLATFORM_UNIX
#include "Unix/BearThread_Unix.h"
#endif

