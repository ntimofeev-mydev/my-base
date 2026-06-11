// #my_engine_source_file
#pragma once

#if !defined(MY_STATIC_RUNTIME)
    #ifdef _MSC_VER
        #ifdef MY_BASE_BUILD
            #define MY_BASE_EXPORT __declspec(dllexport)
        #else
            #define MY_BASE_EXPORT __declspec(dllimport)
        #endif

    #else
        #error Unknown Compiler/OS
    #endif
#else
    #define MY_BASE_EXPORT
#endif
