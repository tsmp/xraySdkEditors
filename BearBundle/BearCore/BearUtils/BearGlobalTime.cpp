#include "BearCore.hpp"
#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#include "Windows/BearGlobalTime_Windows.h"
#elif CURRENT_PLATFORM == PLATFORM_UNIX
#include "Unix/BearGlobalTime_Unix.h"
#endif 
