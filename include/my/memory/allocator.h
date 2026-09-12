// #my_engine_source_file
#pragma once

#include "my/base/config.h"
#include "my/base/platform_defs.h"
#include "my/memory/mem_base.h"
#include "my/rtti/ptr.h"
#include "my/rtti/rtti_object.h"

#include <limits>
#include <memory_resource>
#include <type_traits>

namespace my
{
    /**
     */
    struct MY_ABSTRACT_TYPE IAllocator : IRefCounted
    {
        MY_INTERFACE(my::IAllocator, IRefCounted);

        static inline constexpr size_t kDefaultAlignment = alignof(std::max_align_t);
        static inline constexpr size_t kUnspecifiedValue = std::numeric_limits<size_t>::max();

        [[nodiscard]] virtual void* Alloc(size_t size, size_t alignment = kDefaultAlignment) = 0;

        virtual void Free(void* ptr, size_t size = kUnspecifiedValue, size_t alignment = kUnspecifiedValue) = 0;

        [[nodiscard]] virtual size_t GetMaxAlignment() const = 0;

        [[nodiscard]] virtual std::pmr::memory_resource* GetMemoryResource() = 0;
    };

    struct MY_ABSTRACT_TYPE IReallocAllocator : IAllocator
    {
        MY_INTERFACE(my::IReallocAllocator, IAllocator);

    public:
        [[nodiscard]] virtual void* Realloc(void* oldPtr, size_t size, size_t alignment = kUnspecifiedValue) = 0;
    };

    using AllocatorPtr = Ptr<IAllocator>;

    MY_BASE_EXPORT AllocatorPtr CreateGenericAllocator(bool threadSafe = true);

    MY_BASE_EXPORT IReallocAllocator& GetCrtAllocator();

    MY_BASE_EXPORT IAllocator& GetDefaultAlignedAllocator();

    MY_FORCE_INLINE IAllocator* GetCrtAllocatorPtr()
    {
        return &GetCrtAllocator();
    }

    MY_FORCE_INLINE IAllocator* GetDefaultAlignedAllocatorPtr()
    {
        return &GetDefaultAlignedAllocator();
    }

}  // namespace my

namespace my::mem_detail
{
    /**
        Using Allocator as template parameter (instead if IAllocator) to eliminate virtual calls.
        In general compiler can even inline all m_allocator.XXX calls.
     */
    template <typename AllocatorT>
    class AllocatorMemoryResource final : public std::pmr::memory_resource
    {
    public:
        AllocatorMemoryResource(AllocatorT& allocator) :
            m_allocator{allocator}
        {
            static_assert(std::is_base_of_v<IAllocator, AllocatorT>);
        }

        AllocatorMemoryResource(const AllocatorMemoryResource&) = delete;
        AllocatorMemoryResource& operator=(const AllocatorMemoryResource&) = delete;

        void* do_allocate(size_t size, size_t align) override
        {
            MY_DBG_ASSERT(align > 0 && IsPowerOf2(align));
            MY_DBG_ASSERT(align <= m_allocator.GetMaxAlignment());

            void* ptr = m_allocator.Alloc(size, align);

            MY_FATAL(ptr == nullptr || reinterpret_cast<uintptr_t>(ptr) % align == 0);
            return ptr;
        }

        void do_deallocate(void* ptr, size_t size, size_t align) override
        {
            m_allocator.Free(ptr, size, align);
        }

        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
        {
            return static_cast<const std::pmr::memory_resource*>(this) == &other;
        }

    private:
        AllocatorT& m_allocator;
    };

    template <typename AllocatorImpl, typename AllocatorInterface = IAllocator>
    class AllocatorWithMemResource : public AllocatorInterface
    {
        static_assert(std::is_base_of_v<IAllocator, AllocatorInterface>);

    public:
        std::pmr::memory_resource* GetMemoryResource() final
        {
            return &m_memResource;
        }

    protected:
        AllocatorWithMemResource() :
            m_memResource{static_cast<AllocatorImpl&>(*this)}
        {
        }

    private:
        mutable AllocatorMemoryResource<AllocatorImpl> m_memResource;
    };

    constexpr inline bool IsValidAlignment(size_t align, size_t maxAlign)
    {
        return align == IAllocator::kUnspecifiedValue || (align <= maxAlign && IsPowerOf2(align));
    }

}  // namespace my::mem_detail
