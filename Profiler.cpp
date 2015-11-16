#define _PROFILER_CPP 1
#include "Profiler.h"
#include "mypaths.h"

#ifdef _PROFILE_
Profiler _this_profiler_(string(W_DIR) + string(PROFILE_FILE_NAME));
#endif
