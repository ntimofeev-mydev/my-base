// #my_engine_source_file
#pragma once
#include "my/memory/allocator.h"
#include "my/rtti/ref_counted_class.h"
#include "my/windows/windows_headers.h"

namespace my
{
    class WinHeapAllocator final : public mem_detail::AllocatorWithMemResource<WinHeapAllocator, IReallocAllocator>
    {
        MY_REFCOUNTED_CLASS(my::WinHeapAllocator, IReallocAllocator);

    public:
        WinHeapAllocator(bool threadSafe);

        ~WinHeapAllocator();

        void* Alloc(size_t size, [[maybe_unused]] size_t alignment) override;
        void* Realloc(void* oldPtr, size_t size, size_t alignment) override;
        void Free(void* ptr, [[maybe_unused]] size_t size, size_t alignment) override;
        size_t GetMaxAlignment() const override;

    private:
        HANDLE m_heap = NULL;
    };
}  // namespace my
