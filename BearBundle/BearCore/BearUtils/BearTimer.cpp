#include "BearCore.hpp"
#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#include "Windows/BearTimer_Windows.h"
#elif CURRENT_PLATFORM == PLATFORM_UNIX
#include "Unix/BearTimer_Unix.h"
#endif 

