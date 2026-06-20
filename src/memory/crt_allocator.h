// #my_engine_source_file
#pragma once

#include "my/memory/allocator.h"
#include "my/rtti/ref_counted_class.h"

namespace my
{

    /**
     */
    class CrtAllocator final : public mem_detail::AllocatorWithMemResource<CrtAllocator>
    {
        MY_REFCOUNTED_CLASS(my::CrtAllocator, IAllocator);

    public:
        void* Alloc(size_t size, size_t alignment) override;
        void* Realloc(void* oldPtr, size_t size, size_t alignment) override;
        void Free(void* ptr, size_t size, size_t alignment) override;
        size_t GetMaxAlignment() const override;
    };

    /**
     */
    class AlignedCrtAllocator final : public mem_detail::AllocatorWithMemResource<AlignedCrtAllocator>
    {
        MY_REFCOUNTED_CLASS(my::AlignedCrtAllocator, IAllocator);

    public:
        void* Alloc(size_t size, size_t alignment) override;
        void* Realloc(void* oldPtr, size_t size, size_t alignment) override;
        void Free(void* ptr, size_t size, size_t alignment) override;
        size_t GetMaxAlignment() const override;
    };

}  // namespace my
