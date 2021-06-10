#include "BearCore.hpp"
#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#include "Windows/BearMutex_Windows.h"
#elif CURRENT_PLATFORM == PLATFORM_UNIX
#include "Unix/BearMutex_Unix.h"
#endif

