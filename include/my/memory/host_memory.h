// #my_engine_source_file
#pragma once
#include "my/base/config.h"
#include "my/memory/mem_base.h"
#include "my/rtti/ptr.h"
#include "my/rtti/ref_counted.h"

#include <compare>

// #include <memory>

namespace my
{
    /**
     */
    struct MY_ABSTRACT_TYPE IHostMemory : IRefCounted
    {
        class MY_BASE_EXPORT MemRegion
        {
        public:
            static bool IsAdjacent(const MemRegion& left, const MemRegion& right);

            ~MemRegion();
            MemRegion() = default;
            MemRegion(void* ptr, ByteSize size);
            MemRegion(const MemRegion&) = delete;
            MemRegion(MemRegion&&);

            MemRegion& operator=(const MemRegion&) = delete;
            MemRegion& operator=(MemRegion&&);

            std::strong_ordering operator<=>(const MemRegion&) const noexcept;
            bool operator==(const MemRegion&) const noexcept;
            MemRegion& operator+=(MemRegion&& right) noexcept;

            explicit operator bool() const
            {
                return m_basePtr != nullptr;
            }

            void* GetBasePtr() const
            {
                return m_basePtr;
            }

            ByteSize GetSize() const
            {
                return m_size;
            }

        private:
            void* m_basePtr = nullptr;
            ByteSize m_size = 0;
        };

        virtual ~IHostMemory() = default;

        /**
         */
        virtual MemRegion Alloc(ByteSize size, MemRegion* adjacentRegion = nullptr) = 0;

        /**
         */
        virtual void Free(MemRegion&& pages) = 0;

        /**
         */
        virtual ByteSize GetPageSize() const = 0;

        /**
            @returns Allocation granularity or zero if granularity is not meaningful for that allocator
        */
        virtual ByteSize GetAllocationGranularity() const = 0;
    };

    using HostMemoryPtr = Ptr<IHostMemory>;

    MY_BASE_EXPORT HostMemoryPtr CreateHostVirtualMemory(ByteSize maxSize, ByteSize commitSize, bool threadSafe);

    MY_BASE_EXPORT HostMemoryPtr CreateDefaultHostMemory(bool threadSafe = true);

}  // namespace my
