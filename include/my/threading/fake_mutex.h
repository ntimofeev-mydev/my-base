// #my_engine_source_file
#pragma once
#include "my/threading/thread_safe_annotations.h"

namespace my::threading
{
    /**
     */
    class THREAD_CAPABILITY("mutex") FakeMutex
    {
    public:
        FakeMutex() = default;
        FakeMutex(const FakeMutex&) = delete;
        FakeMutex& operator=(const FakeMutex&) = delete;

        void lock() THREAD_ACQUIRE()
        {
        }

        void unlock() THREAD_RELEASE()
        {
        }
    };
}  // namespace my::threading

