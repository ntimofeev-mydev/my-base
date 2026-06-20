// #my_engine_source_file
#include "crt_allocator.h"

namespace my
{
    using namespace my::mem_detail;

    namespace
    {
        constexpr size_t kMaxSpecificAlignment = 32;
    }

    void* CrtAllocator::Alloc(size_t size, [[maybe_unused]] size_t alignment)
    {
        static_assert(kDefaultAlignment <= alignof(std::max_align_t));

        MY_DBG_FATAL(alignment != 0 && IsPowerOf2(alignment) && alignment <= kDefaultAlignment);

        void* const ptr = ::malloc(size);
        MY_DBG_FATAL(reinterpret_cast<uintptr_t>(ptr) % alignment == 0);

        return ptr;
    }

    void* CrtAllocator::Realloc(void* oldPtr, size_t size, [[maybe_unused]] size_t alignment)
    {
        MY_DBG_FATAL(IsValidAlignment(alignment, kDefaultAlignment));
        return ::realloc(oldPtr, size);
    }

    void CrtAllocator::Free(void* ptr, [[maybe_unused]] size_t size, [[maybe_unused]] size_t alignment)
    {
        MY_DBG_FATAL(IsValidAlignment(alignment, kDefaultAlignment));
        ::free(ptr);
    }

    size_t CrtAllocator::GetMaxAlignment() const
    {
        return kDefaultAlignment;
    }

    void* AlignedCrtAllocator::Alloc(size_t size, size_t alignment)
    {
        MY_DBG_FATAL(alignment != 0 && IsPowerOf2(alignment));

#ifdef _WIN32
        return ::_aligned_malloc(size, alignment);
#else
        return std::aligned_alloc(alignment, alignedSize(size, alignment));
#endif
    }

    void* AlignedCrtAllocator::Realloc(void* oldPtr, size_t size, [[maybe_unused]] size_t alignment)
    {
        MY_DBG_FATAL(alignment != kUnspecifiedValue, "Alignment MUST BE explicitly specified");
        MY_DBG_FATAL(alignment != 0 && IsPowerOf2(alignment));
#ifdef _WIN32
        auto const ptr = ::_aligned_realloc(oldPtr, size, alignment);
        return ptr;
#else
        return ::realloc(oldPtr, size);
#endif
    }

    void AlignedCrtAllocator::Free(void* ptr, [[maybe_unused]] size_t size, [[maybe_unused]] size_t alignment)
    {
#ifdef _WIN32
        ::_aligned_free(ptr);
#else
        ::free(ptr);
#endif
    }

    size_t AlignedCrtAllocator::GetMaxAlignment() const
    {
        return kMaxSpecificAlignment;
    }

}  // namespace my
