// #my_engine_source_file

#include "my/diag/assert.h"
#include "my/memory/block_allocator.h"
#include "my/rtti/ref_counted_class.h"

/**
THREAD_YIELD
#if defined(_MSC_VER)
            _mm_pause();
#elif defined(__GNUC__) || defined(__clang__)
            __builtin_ia32_pause();
#endif
*/

using namespace my::mem_detail;

namespace my
{
    namespace
    {
        struct Block
        {
            Block* next;
        };

        constexpr size_t kPreAllocateBlockCount = 20;
        constexpr size_t kBlockAlignment = 16;

    }  // namespace

    /**
     */
    class ThreadSafeBlockAllocator final : public mem_detail::AllocatorWithMemResource<ThreadSafeBlockAllocator>
    {
        MY_REFCOUNTED_CLASS(my::ThreadSafeBlockAllocator, IAllocator);

    public:
        ThreadSafeBlockAllocator(HostMemoryPtr&& memory, size_t blockSize) :
            m_memory{std::move(memory)},
            m_blockSize{AlignedSize(blockSize, kBlockAlignment)}
        {
            MY_DBG_FATAL(m_memory);
            m_pages.emplace_back(m_memory->Alloc(m_blockSize * kPreAllocateBlockCount));
        }

        void* Alloc([[maybe_unused]] size_t size, [[maybe_unused]] size_t align) override
        {

            MY_DBG_FATAL(size <= m_blockSize, "Request to alloc({}) bytes, but block size = ({}) bytes", size, m_blockSize);
            MY_DBG_FATAL(IsValidAlignment(align, kBlockAlignment), "Invalid alignment ({})", align);

            const std::lock_guard lock{m_allocMutex};
            if (m_freeBlock)
            {
                return std::exchange(m_freeBlock, m_freeBlock->next);
            }

            MY_DBG_FATAL(!m_pages.empty());

            IHostMemory::MemRegion* pages = &m_pages.back();

            if (m_allocOffset + m_blockSize > pages->GetSize())
            {
                IHostMemory::MemRegion& newPages = m_pages.emplace_back(m_memory->Alloc(m_blockSize * kPreAllocateBlockCount));
                MY_ASSERT(newPages, "Fail to allocate more pages");
                if (!newPages)
                {
                    return nullptr;
                }

                pages = &newPages;
                m_allocOffset = 0;
            }

            MY_FATAL(pages);
            MY_DBG_FATAL(m_allocOffset + m_blockSize <= pages->GetSize());

            const size_t alloc_offset = std::exchange(m_allocOffset, m_allocOffset + m_blockSize);

            std::byte* const ptr = reinterpret_cast<std::byte*>(pages->GetBasePtr()) + alloc_offset;
            MY_DBG_FATAL(reinterpret_cast<uintptr_t>(ptr) % kBlockAlignment == 0);

            return ptr;
        }

        void Free(void* ptr, [[maybe_unused]] size_t size, [[maybe_unused]] size_t align) override
        {
            MY_DBG_FATAL(size == IAllocator::kUnspecifiedValue || size <= m_blockSize);
            if (!ptr)
            {
                return;
            }

            const std::lock_guard lock {m_allocMutex};
            Block* block = reinterpret_cast<Block*>(ptr);
            block->next = m_freeBlock;
            m_freeBlock = block;

        }

        size_t GetMaxAlignment() const override
        {
            return kBlockAlignment;
        }

    private:
        HostMemoryPtr m_memory;
        const size_t m_blockSize;
        std::vector<IHostMemory::MemRegion> m_pages;
        size_t m_allocOffset = 0;
        Block* m_freeBlock = nullptr;
        std::mutex m_allocMutex;
    };

    AllocatorPtr CreateBlockAllocator(HostMemoryPtr hostMemory, ByteSize blockSize, bool threadSafe)
    {
        MY_DBG_ASSERT(threadSafe, "Thread unsafe allocator is not implemented");

        if (threadSafe)
        {
            return rtti::CreateInstance<ThreadSafeBlockAllocator>(std::move(hostMemory), blockSize);
        }

        return rtti::CreateInstance<ThreadSafeBlockAllocator>(std::move(hostMemory), blockSize);
    }

}  // namespace my
