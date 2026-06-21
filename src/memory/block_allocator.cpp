// #my_engine_source_file

#include "my/memory/block_allocator.h"
#include "my/rtti/ref_counted_class.h"
#include "my/threading/fake_mutex.h"

using namespace my::mem_detail;

namespace my
{
    namespace
    {
        constexpr size_t kPreAllocateBlockCount = 10;
    }  // namespace

    /**
     *
     */
    class Pool
    {
    public:
        static inline constexpr size_t kBlockAlignment = 16;

        Pool(IHostMemory& mem, size_t blockSize) :
            m_memory(mem),
            m_blockSize(blockSize)
        {
            m_pages.emplace_back(m_memory.Alloc(blockSize * kPreAllocateBlockCount));
        }

        size_t GetBlockSize() const
        {
            return m_blockSize;
        }

        void* Allocate()
        {
            void* ptr = nullptr;

            if (!m_freeBlocksBegin)
            {
                ptr = AllocateNewBlock();
            }
            else
            {
                Block* const block = reinterpret_cast<Block*>(m_freeBlocksBegin);
                m_freeBlocksBegin = block->next;
                ptr = block;
            }

#ifndef NDEBUG
            if (ptr)
            {
                memset(ptr, 0, m_blockSize);
            }
#endif

            return ptr;
        }

        void Free(void* ptr)
        {
            reinterpret_cast<Block*>(ptr)->next = m_freeBlocksBegin;
            m_freeBlocksBegin = ptr;
        }

    private:
        struct MemRegionEntry
        {
            IHostMemory::MemRegion pages;
            size_t offset = 0;
        };

        struct Block
        {
            void* next;
        };

        void* AllocateNewBlock()
        {
            MY_DBG_ASSERT(!m_pages.empty());

            MemRegionEntry* region = &m_pages.back();
            if (const size_t availSize = region->pages.GetSize() - region->offset; availSize < m_blockSize)
            {
                IHostMemory::MemRegion memPages = m_memory.Alloc(m_blockSize);
                MY_DBG_FATAL(memPages, "Fail to allocate more pages");
                if (!memPages)
                {
                    return nullptr;
                }

                region = &m_pages.emplace_back(std::move(memPages));
            }

            MY_DBG_FATAL(region->pages.GetSize() >= m_blockSize);

            const size_t allocOffset = std::exchange(region->offset, region->offset + m_blockSize);
            return reinterpret_cast<std::byte*>(region->pages.GetBasePtr()) + allocOffset;
        }

        std::vector<MemRegionEntry> m_pages;
        IHostMemory& m_memory;
        const size_t m_blockSize;
        void* m_freeBlocksBegin = nullptr;
    };

    /**
     *
     */
    template <typename Mutex>
    class BlockAllocator final : public mem_detail::AllocatorWithMemResource<BlockAllocator<Mutex>>
    {
    public:
        MY_REFCOUNTED_CLASS(BlockAllocator, IAllocator);

        BlockAllocator(HostMemoryPtr memory, size_t blockSize) :
            m_memory(memory),
            m_pool{*memory, blockSize}
        {
        }

        // ~FixedSizeBlockAllocator()
        // {
        //     // #ifndef NDEBUG
        //     //             std::stringstream ss;
        //     //             ss << std::format("[Allocator]\nClear pool allocator, total pools: ({})\n", m_pools.size());

        //     //             for(auto& pool : m_pools)
        //     //             {
        //     //                 ss << std::format("* [{0}], size = {1} bytes", pool.getBlockSize(), pool.getSize());
        //     //             }
        //     //             //LOG_DEBUG(ss.str());
        //     // #endif
        // }

        void* Alloc(size_t size, [[maybe_unused]] size_t align) override
        {
            MY_DBG_FATAL(size <= m_pool.GetBlockSize());
            MY_DBG_FATAL(IsValidAlignment(align, Pool::kBlockAlignment), "Invalid alignment ({})", align);

            const std::lock_guard lock(m_mutex);

            void* const ptr = m_pool.Allocate();
            MY_DBG_ASSERT(reinterpret_cast<uintptr_t>(ptr) % Pool::kBlockAlignment == 0);

            return ptr;
        }

        void* Realloc(void* oldPtr, size_t size, [[maybe_unused]] size_t align) override
        {
            MY_DBG_FATAL(size <= m_pool.GetBlockSize());
            MY_DBG_FATAL(IsValidAlignment(align, Pool::kBlockAlignment), "Invalid alignment ({})", align);

            if (oldPtr)
            {
                // MY_DBG_FATAL(m_pool
                return oldPtr;
            }

            const std::lock_guard lock(m_mutex);
            void* const ptr = m_pool.Allocate();
            MY_DBG_ASSERT(reinterpret_cast<uintptr_t>(ptr) % Pool::kBlockAlignment == 0);

            return ptr;
        }

        size_t GetMaxAlignment() const override
        {
            return Pool::kBlockAlignment;
        }

        // void* reallocAligned(void* oldPtr, size_t size, [[maybe_unused]] size_t alignment) override
        // {
        //     MY_DBG_FATAL(isPowerOf2(alignment));

        //     void* const newPtr = this->realloc(oldPtr, size);

        //     MY_DBG_FATAL(newPtr == nullptr || reinterpret_cast<uintptr_t>(newPtr) % alignment == 0);

        //     return newPtr;
        // }

        void Free(void* ptr, size_t size, [[maybe_unused]] size_t align) override
        {
            MY_DBG_FATAL(size == IAllocator::kUnspecifiedValue || size <= m_pool.GetBlockSize());
            MY_DBG_FATAL(IsValidAlignment(align, Pool::kBlockAlignment));

            if (ptr)
            {
                const std::lock_guard lock(m_mutex);
                m_pool.Free(ptr);
            }
        }

        // void freeAligned(void* ptr, size_t) override
        // {
        //     free(ptr);
        // }

    private:
        HostMemoryPtr m_memory;
        Pool m_pool;
        Mutex m_mutex;
    };

    AllocatorPtr CreateBlockAllocator(HostMemoryPtr hostMemory, ByteSize blockSize, bool threadSafe)
    {
        const size_t alignedBlockSize = AlignedSize(blockSize, Pool::kBlockAlignment);

        using ThreadSafeAllocator = BlockAllocator<std::mutex>;
        using ThreadUnsafeAllocator = BlockAllocator<threading::FakeMutex>;

        if (threadSafe)
        {
            return rtti::CreateInstance<ThreadSafeAllocator>(std::move(hostMemory), alignedBlockSize);
        }

        return rtti::CreateInstance<ThreadUnsafeAllocator>(std::move(hostMemory), alignedBlockSize);
    }

}  // namespace my
