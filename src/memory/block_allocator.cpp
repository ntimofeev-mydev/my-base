// #my_engine_source_file

#include "my/diag/assert.h"
#include "my/memory/block_allocator.h"
#include "my/rtti/ref_counted_class.h"

#include <iostream>

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
        constexpr size_t kPreAllocateBlockCount = 20;
        static inline constexpr size_t kBlockAlignment = 16;

        struct Block
        {
            Block* next;
        };
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

            Block* block = m_freeBlock.load(std::memory_order_relaxed);
            while (block && !m_freeBlock.compare_exchange_weak(block, block->next, std::memory_order_acquire, std::memory_order_relaxed))
            {
            }

            if (block)
            {
                return block;
            }

            const std::lock_guard lock{m_allocMutex};
            MY_DBG_FATAL(!m_pages.empty());

            IHostMemory::MemRegion* pages = &m_pages.back();

            if (m_allocOffset + m_blockSize > pages->GetSize())
            {
                IHostMemory::MemRegion& newPages = m_pages.emplace_back(m_memory->Alloc(m_blockSize * kPreAllocateBlockCount));
                MY_DBG_FATAL(newPages, "Fail to allocate more pages");
                if (!newPages)
                {
                    return nullptr;
                }

                pages = &newPages;
                m_allocOffset = 0;
            }

            MY_DBG_FATAL(pages);
            MY_DBG_FATAL(m_allocOffset + m_blockSize <= pages->GetSize());

            const size_t allocOffset = std::exchange(m_allocOffset, m_allocOffset + m_blockSize);

            std::byte* const ptr = reinterpret_cast<std::byte*>(pages->GetBasePtr()) + allocOffset;

            MY_DBG_ASSERT(reinterpret_cast<uintptr_t>(ptr) % kBlockAlignment == 0);

            return ptr;
        }

        void* Realloc(void* ptr, size_t size, size_t align) override
        {
            if (ptr)
            {
                MY_DBG_FATAL(size <= m_blockSize, "Request to alloc({}) bytes, but block size = ({}) bytes", size, m_blockSize);
                MY_DBG_FATAL(IsValidAlignment(align, kBlockAlignment), "Invalid alignment ({})", align);
                // TODO: debug check owns ptr;

                return ptr;
            }

            return Alloc(size, align);
        }

        void Free(void* ptr, [[maybe_unused]] size_t size, [[maybe_unused]] size_t align) override
        {
            MY_DBG_FATAL(size == IAllocator::kUnspecifiedValue || size <= m_blockSize);

            if (ptr)
            {
                Block* block = reinterpret_cast<Block*>(ptr);
                block->next = m_freeBlock.load(std::memory_order_relaxed);
                while (!m_freeBlock.compare_exchange_weak(block->next, block, std::memory_order_release, std::memory_order_relaxed))
                {
                }
            }
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
        std::atomic<Block*> m_freeBlock{nullptr};
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
