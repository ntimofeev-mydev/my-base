// #my_engine_source_file
#pragma once

// #include <cstdlib>
#include "my/base/config.h"

#ifdef _WIN32
    #include <intrin.h>
#else
    #error unsupported os
#endif

#ifdef _WIN32

    #define MY_DEBUG_BREAK (__nop(), __debugbreak())
// #define MY_PLATFORM_ABORT (std::abort())

#endif

// #include "my/base/config.h"
// #include "my/utils/preprocessor.h"

// //#include MY_PLATFORM_HEADER(platform_debug.h)

namespace my::debug {

    MY_BASE_EXPORT
    bool IsRunningUnderDebugger();

}  // namespace my::debug
