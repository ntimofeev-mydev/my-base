// #my_engine_source_file
#pragma once

#include "my/base/config.h"
#include "my/base/platform_defs.h"
#include "my/memory/mem_base.h"
#include "my/rtti/ptr.h"
#include "my/rtti/rtti_object.h"

// #include <concepts>
#include <limits>
#include <memory_resource>
#include <type_traits>

// #include "my/utils/type_utility.h"

namespace my
{
    // struct IAllocator;

    struct MY_ABSTRACT_TYPE IAllocator : IRefCounted
    {
        MY_INTERFACE(my::IAllocator, IRefCounted);

        static inline constexpr size_t kDefaultAlignment = alignof(std::max_align_t);
        static inline constexpr size_t kUnspecifiedValue = std::numeric_limits<size_t>::max();

        [[nodiscard]] virtual void* Alloc(size_t size, size_t alignment = kDefaultAlignment) = 0;

        [[nodiscard]] virtual void* Realloc(void* oldPtr, size_t size, size_t alignment = kUnspecifiedValue) = 0;

        virtual void Free(void* ptr, size_t size = kUnspecifiedValue, size_t alignment = kUnspecifiedValue) = 0;

        virtual size_t GetMaxAlignment() const = 0;

        virtual std::pmr::memory_resource* GetMemoryResource() = 0;

        virtual void SetName(const char*)
        {
        }

        virtual const char* GetName() const
        {
            return "";
        }
    };



    using AllocatorPtr = Ptr<IAllocator>;

    MY_BASE_EXPORT AllocatorPtr CreateDefaultGenericAllocator(bool threadSafe = true);

    MY_BASE_EXPORT IAllocator& GetDefaultAllocator();

    MY_BASE_EXPORT IAllocator& GetDefaultAlignedAllocator();

    MY_FORCE_INLINE IAllocator* GetDefaultAllocatorPtr()
    {
        return &GetDefaultAllocator();
    }

    MY_FORCE_INLINE IAllocator* GetDefaultAlignedAllocatorPtr()
    {
        return &GetDefaultAlignedAllocator();
    }

}  // namespace my

namespace my::mem_detail
{
    /**
     */
    template <typename Allocator>
    class AllocatorMemoryResource final : public std::pmr::memory_resource
    {
    public:
        AllocatorMemoryResource(Allocator& allocator) :
            m_allocator{allocator}
        {
            static_assert(std::is_base_of_v<IAllocator, Allocator>);
        }

        AllocatorMemoryResource(const AllocatorMemoryResource&) = delete;
        AllocatorMemoryResource& operator=(const AllocatorMemoryResource&) = delete;

        void* do_allocate(size_t size, size_t align) override
        {
            MY_DBG_ASSERT(align > 0 && IsPowerOf2(align));
            MY_DBG_ASSERT(align <= m_allocator.GetMaxAlignment());

            void* ptr = m_allocator.alloc(size, align);

            MY_FATAL(ptr == nullptr || reinterpret_cast<uintptr_t>(ptr) % align == 0);
            return ptr;
        }

        void do_deallocate(void* ptr, size_t size, size_t align) override
        {
            m_allocator.free(ptr, size, align);
        }

        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return static_cast<const std::pmr::memory_resource*>(this) == &other;
        }

    private:
        Allocator& m_allocator;
    };

    template <typename T>
    class AllocatorWithMemResource : public IAllocator
    {
        MY_INTERFACE(my::mem_detail::AllocatorWithMemResource<T>, IAllocator);
    public:
        
        std::pmr::memory_resource* GetMemoryResource() final
        {
            return &m_memResource;
        }

    protected:
        AllocatorWithMemResource() :
            m_memResource{static_cast<T&>(*this)}
        {
        }

    private:
        mutable AllocatorMemoryResource<T> m_memResource;
    };

    constexpr inline bool IsValidAlignment(size_t align, size_t maxAlign)
    {
        return align == IAllocator::kUnspecifiedValue || (align <= maxAlign && IsPowerOf2(align));
    }

}  // namespace my::mem_detail