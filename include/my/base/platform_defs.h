// #my_engine_source_file
#pragma once

#ifdef _WIN32
    #if defined(_MSC_VER) || defined(__clang__)
        #define MY_NOVTABLE __declspec(novtable)
    #else
        #define MY_NOVTABLE
    #endif

#else
    #define MY_NOVTABLE
#endif

#define MY_ABSTRACT_TYPE MY_NOVTABLE


#if defined(_MSC_VER)
    #define MY_FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define MY_FORCE_INLINE inline __attribute__((always_inline))
#else
    #define MY_FORCE_INLINE inline
#endif
