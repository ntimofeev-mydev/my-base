// #my_engine_source_file
#include "my/memory/host_memory.h"
#include "my/rtti/ref_counted_class.h"

#include <atomic>
#include <mutex>

namespace my
{
    class WinHostVirtualMemory final : public IHostMemory
    {
        MY_REFCOUNTED_CLASS(my::WinHostVirtualMemory, IHostMemory);

    public:
        WinHostVirtualMemory(const WinHostVirtualMemory&) = delete;
        WinHostVirtualMemory& operator=(const WinHostVirtualMemory&) = delete;

        WinHostVirtualMemory(ByteSize size, ByteSize commitSize);
        ~WinHostVirtualMemory();

    private:
        bool OwnsRegion(const MemRegion& r) const;
        MemRegion Alloc(ByteSize size, MemRegion* adjacentRegion) override;
        void Free(MemRegion&&) override;
        ByteSize GetPageSize() const override;
        ByteSize GetAllocationGranularity() const override;

        const size_t m_size;
        std::atomic<size_t> m_commitedSize = 0;
        std::atomic<size_t> m_allocOffset = 0;
        std::byte* m_basePtr;
        std::mutex m_mutex;
    };

}  // namespace my
