// #my_engine_source_file
#include "crt_allocator.h"

#if defined(_WIN32)
    #include "windows/win_heap_allocator.h"
#else
    #error add OS specific
#endif

namespace my
{
    IAllocator& GetDefaultAllocator()
    {
        static Ptr<CrtAllocator> allocator = rtti::CreateInstanceSingleton<CrtAllocator>();
        return *allocator;
    }

    IAllocator& GetDefaultAlignedAllocator()
    {
        static Ptr<AlignedCrtAllocator> allocator = rtti::CreateInstanceSingleton<AlignedCrtAllocator>();
        return *allocator;
    }

    AllocatorPtr CreateGenericAllocator(bool threadSafe)
    {
#if defined(_WIN32)
        return rtti::CreateInstance<WinHeapAllocator>(threadSafe);
#endif
    }

}  // namespace my
