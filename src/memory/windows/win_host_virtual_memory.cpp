// #my_engine_source_file
#include "my/diag/assert.h"
#include "win_host_virtual_memory.h"

namespace my
{
    WinHostVirtualMemory::WinHostVirtualMemory(ByteSize size, ByteSize commitSize) :
        m_size(AlignedSize(size.GetByteCount(), mem::kAllocationGranularity))
    {
        m_basePtr = reinterpret_cast<std::byte*>(::VirtualAlloc(nullptr, static_cast<SIZE_T>(m_size), MEM_RESERVE, PAGE_READWRITE));
        MY_FATAL(m_basePtr);

        if (commitSize > 0)
        {
            m_commitedSize = AlignedSize(commitSize.GetByteCount(), mem::kPageSize);
            ::VirtualAlloc(m_basePtr, static_cast<SIZE_T>(m_commitedSize), MEM_COMMIT, PAGE_READWRITE);
        }
    }

    WinHostVirtualMemory::~WinHostVirtualMemory()
    {
        ::VirtualFree(m_basePtr, 0, MEM_FREE);
    }

    bool WinHostVirtualMemory::OwnsRegion(const MemRegion& r) const
    {
        const std::byte* const ptr = reinterpret_cast<const std::byte*>(r.GetBasePtr());
        return m_basePtr <= ptr && ((ptr + r.GetSize()) < (m_basePtr + m_size));
    }

    IHostMemory::MemRegion WinHostVirtualMemory::Alloc(ByteSize size, MemRegion* adjacentRegion)
    {
        const size_t regionByteSize = AlignedSize(size.GetByteCount(), mem::kPageSize);
        size_t regionStart = m_allocOffset;

        for (; !m_allocOffset.compare_exchange_strong(regionStart, m_allocOffset + regionByteSize, std::memory_order_relaxed);)
        {
        }

        const size_t requiredCommitedSize = regionStart + regionByteSize;

        if (m_commitedSize.load(std::memory_order_relaxed) < requiredCommitedSize)
        {
            const std::lock_guard lock(m_mutex);

            const size_t currentCommitedSize = m_commitedSize.load(std::memory_order_relaxed);

            if (currentCommitedSize < requiredCommitedSize)
            {
                const size_t commitSize = requiredCommitedSize - currentCommitedSize;
                MY_DBG_ASSERT(commitSize % mem::kPageSize == 0);

                const size_t newCommitedSize = currentCommitedSize + commitSize;
                if (newCommitedSize > m_size)
                {
                    return MemRegion{};
                }

                ::VirtualAlloc(m_basePtr + currentCommitedSize, static_cast<SIZE_T>(commitSize), MEM_COMMIT, PAGE_READWRITE);
                m_commitedSize.store(newCommitedSize, std::memory_order_relaxed);
            }
        }

        MemRegion allocatedRegion{m_basePtr + regionStart, regionByteSize};

        if (adjacentRegion)
        {
            MY_DBG_FATAL(OwnsRegion(*adjacentRegion));
            MY_DBG_FATAL(MemRegion::IsAdjacent(*adjacentRegion, allocatedRegion), "Adjacent regions can only be allocated sequentially.");

            MemRegion r {std::move(*adjacentRegion)};
            r += std::move(allocatedRegion);
            return r;
        }

        return allocatedRegion;
    }

    void WinHostVirtualMemory::Free([[maybe_unused]] MemRegion&& region)
    {
        MY_ASSERT(diag::kForceFail, "WindowsHostVirtualMemory does not support page deallocation.");
    }

    ByteSize WinHostVirtualMemory::GetPageSize() const
    {
        return mem::kPageSize;
    }

    ByteSize WinHostVirtualMemory::GetAllocationGranularity() const
    {
        return mem::kPageSize;
    }

}  // namespace my

