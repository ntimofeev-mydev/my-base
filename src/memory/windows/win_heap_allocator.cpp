// #my_engine_source_file
#include "my/diag/assert.h"
#include "my/diag/windows/win_error.h"
#include "win_heap_allocator.h"

// #include "my/rtti/rtti_impl.h"

using namespace my::mem_detail;

namespace my
{
    namespace
    {
        static inline constexpr DWORD GetAllocOpts()
        {
            constexpr DWORD opts =
#ifndef NDEBUG
                HEAP_ZERO_MEMORY;
#else
                0;
#endif
            return opts;
        }
    }

    WinHeapAllocator::WinHeapAllocator(bool threadSafe)
    {
        const DWORD opts = threadSafe ? 0 : HEAP_NO_SERIALIZE;
        const SIZE_T initialSize = mem::kAllocationGranularity;

        m_heap = ::HeapCreate(opts, initialSize, 0);
        MY_FATAL(m_heap != NULL, diag::GetWinErrorMessageA(diag::GetAndResetLastWinError()));
    }

    WinHeapAllocator::~WinHeapAllocator()
    {
        ::HeapDestroy(m_heap);
    }

    void* WinHeapAllocator::Alloc(size_t size, [[maybe_unused]] size_t alignment)
    {
        MY_DBG_FATAL(m_heap != NULL);
        MY_DBG_FATAL(IsValidAlignment(alignment, MEMORY_ALLOCATION_ALIGNMENT));

        void* const ptr = ::HeapAlloc(m_heap, GetAllocOpts(), size);
        MY_DBG_FATAL(reinterpret_cast<uintptr_t>(ptr) % alignment == 0);
        return ptr;
    }

    void* WinHeapAllocator::Realloc(void* oldPtr, size_t size, size_t alignment)
    {
        MY_DBG_FATAL(m_heap != NULL);
        MY_DBG_FATAL(IsValidAlignment(alignment, MEMORY_ALLOCATION_ALIGNMENT));

        return ::HeapReAlloc(m_heap, GetAllocOpts(), oldPtr, size);
    }

    void WinHeapAllocator::Free(void* ptr, [[maybe_unused]] size_t size, size_t alignment)
    {
        MY_DBG_FATAL(m_heap != NULL);
        MY_DBG_FATAL(IsValidAlignment(alignment, MEMORY_ALLOCATION_ALIGNMENT));

        ::HeapFree(m_heap, 0, ptr);
    }

    size_t WinHeapAllocator::GetMaxAlignment() const
    {
        return MEMORY_ALLOCATION_ALIGNMENT;
    }

}  // namespace my
