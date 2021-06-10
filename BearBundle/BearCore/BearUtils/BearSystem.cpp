#include "BearCore.hpp"
#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#include "Windows/BearSystem_Windows.h"
#elif CURRENT_PLATFORM == PLATFORM_UNIX
#include "Unix/BearSystem_Unix.h"
#endif

