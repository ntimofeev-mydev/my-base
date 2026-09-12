// #my_engine_source_file

#ifdef _WIN32
    #include "my/debug/debugger.h"

    #include <crtdbg.h>
    #include <debugapi.h>
#else
    #error "Os"
#endif

namespace my::debug {

#ifdef _WIN32
    bool IsRunningUnderDebugger()
    {
        return ::IsDebuggerPresent() == TRUE;
    }
#else

#endif

}  // namespace my::debug