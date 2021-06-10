#include "BearCore.hpp" 
#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#include "Windows/BearManagerProjects_Windows.h"
#elif CURRENT_PLATFORM == PLATFORM_UNIX
#include "Unix/BearManagerProjects_Unix.h"
#endif 

