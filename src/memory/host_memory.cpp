// #my_engine_source_file
#include "my/diag/assert.h"
#include "my/memory/host_memory.h"
#include "my/rtti/ref_counted_class.h"

#ifdef _WIN32
    #include "windows/win_host_virtual_memory.h"
#else
    #error implement OS specific
#endif

#include <cstdlib>

namespace my
{
    IHostMemory::MemRegion::~MemRegion()
    {
    }

    IHostMemory::MemRegion::MemRegion(void* ptr, ByteSize size) :
        m_basePtr(ptr),
        m_size(size)
    {
        MY_DBG_ASSERT(m_basePtr == nullptr || m_size > 0);
    }

    IHostMemory::MemRegion::MemRegion(MemRegion&& other) :
        m_basePtr(std::exchange(other.m_basePtr, nullptr)),
        m_size(std::exchange(other.m_size, 0))
    {
    }

    IHostMemory::MemRegion& IHostMemory::MemRegion::operator=(MemRegion&& other)
    {
        m_basePtr = std::exchange(other.m_basePtr, nullptr);
        m_size = std::exchange(other.m_size, 0);

        return *this;
    }

    std::strong_ordering IHostMemory::MemRegion::operator<=>(const MemRegion& other) const noexcept
    {
        MY_DBG_ASSERT(m_basePtr != other.m_basePtr || m_size == other.m_size);
        return reinterpret_cast<uintptr_t>(m_basePtr) <=> reinterpret_cast<uintptr_t>(other.m_basePtr);
    }

    bool IHostMemory::MemRegion::operator==(const MemRegion& other) const noexcept
    {
        MY_DBG_ASSERT(m_basePtr != other.m_basePtr || m_size == other.m_size);
        return m_basePtr == other.m_basePtr;
    }

    IHostMemory::MemRegion& IHostMemory::MemRegion::operator+=(MemRegion&& right) noexcept
    {
        // TODO: check that regions concatenation is supported pn host memory.
        MY_DBG_ASSERT(right);
        MY_DBG_ASSERT(IsAdjacent(*this, right));
        MY_DBG_ASSERT(m_basePtr < right.m_basePtr);

        m_size = m_size + right.m_size;
        right.m_size = 0;
        right.m_basePtr = nullptr;

        return *this;
    }

    bool IHostMemory::MemRegion::IsAdjacent(const MemRegion& left, const MemRegion& right)
    {
        if (!left || !right)
        {
            return false;
        }

        const auto res = left <=> right;

        if (res == std::strong_ordering::greater)
        {
            return reinterpret_cast<const std::byte*>(right.m_basePtr) + right.m_size == reinterpret_cast<const std::byte*>(left.m_basePtr);
        }
        else if (res == std::strong_ordering::less)
        {
            return reinterpret_cast<const std::byte*>(left.m_basePtr) + left.m_size == reinterpret_cast<const std::byte*>(right.m_basePtr);
        }

        return false;
    }
#if 0
    class HostCrtMemory final : public IHostMemory
    {
        MY_REFCOUNTED_CLASS(my::HostCrtMemory, IHostMemory);

        static constexpr size_t kMinBlockAlignment = 128;

    public:
        HostCrtMemory() = default;

        ~HostCrtMemory()
        {
        }

    private:
        MemRegion AllocPages(size_t size, [[maybe_unused]] MemRegion* adjacentRegion) override
        {
            MY_DBG_FATAL(!adjacentRegion, "Crt Host Memory does not support allocation for adjacent region");

            size = AlignedSize(size, mem::kPageSize);
    #ifdef _WIN32
            void* const ptr = ::_aligned_malloc(size, kMinBlockAlignment);
    #else
            void* const ptr = std::aligned_alloc(kGuaranteedBlockAlignment, size);
    #endif
            MY_DBG_FATAL(reinterpret_cast<uintptr_t>(ptr) % kMinBlockAlignment == 0);

            return MemRegion{ptr, size};
        }

        void FreePages(MemRegion&& pages) override
        {
    #ifdef _WIN32
            ::_aligned_free(pages.GetBasePtr());
    #else
            std::free(pages.GetBasePtr());
    #endif
        }

        ByteSize GetPageSize() const override
        {
            return mem::kPageSize;
        }

        ByteSize GetAllocationGranularity() const override
        {
            return mem::kPageSize;
        }
    };
#endif

    // HostMemoryPtr CreateCrtHostMemory([[maybe_unused]] bool threadSafe)
    // {
    //     return rtti::CreateInstanceSingleton<HostCrtMemory>();
    // }

#ifdef _WIN32
    HostMemoryPtr CreateHostVirtualMemory(ByteSize maxSize, ByteSize commitSize, [[maybe_unused]] bool threadSafe)
    {
        return rtti::CreateInstance<WinHostVirtualMemory>(maxSize, commitSize);
    }
#endif
}  // namespace my
