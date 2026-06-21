// #my_engine_source_file
#include "my/memory/host_memory.h"
#include "my/threading/barrier.h"

using namespace my::my_literals;

namespace my::test
{
    /**
     */
    TEST(HostMemory, Allocate)
    {
        auto hostMemory = CreateHostVirtualMemory(mem::kAllocationGranularity * 2, 0, true);
        auto pages = hostMemory->Alloc(1_b);

        ASSERT_TRUE(pages);
        ASSERT_EQ(pages.GetSize(), hostMemory->GetPageSize());
    }

    /**
     */
    TEST(HostMemory, AllocateAdjacent)
    {
        auto hostMemory = CreateHostVirtualMemory(mem::kAllocationGranularity * 2, 0, true);
        auto pages1 = hostMemory->Alloc(mem::kPageSize * 2);

        const void* const ptr1 = pages1.GetBasePtr();

        auto pages2 = hostMemory->Alloc(mem::kPageSize, &pages1);

        ASSERT_FALSE(pages1);
        ASSERT_EQ(pages2.GetSize(), mem::kPageSize * 3);
        ASSERT_EQ(pages2.GetBasePtr(), ptr1);
    }

    /**
     */
    TEST(HostMemory, AllocateMultiThread)
    {
        constexpr size_t kThreadCount = 15;
        constexpr size_t kIterationCount = 250;
        constexpr auto kRequireMinimumMemory = mem::kPageSize * kThreadCount * kIterationCount;

        using MemRegions = std::vector<IHostMemory::MemRegion>;

        std::vector<std::thread> threads;
        std::vector<MemRegions> threadAllocatedRegion;

        threads.reserve(kThreadCount);
        threadAllocatedRegion.reserve(kThreadCount);

        auto hostMemory = CreateHostVirtualMemory(kRequireMinimumMemory, 0, true);

        threading::Barrier barrier(kThreadCount);

        for (size_t i = 0; i < kThreadCount; ++i)
        {
            threads.emplace_back([](threading::Barrier& barrier, HostMemoryPtr hostMemory, MemRegions& pageCollection)
            {
                pageCollection.reserve(kIterationCount);
                barrier.Enter();

                for (size_t i = 0; i < kIterationCount; ++i)
                {
                    pageCollection.emplace_back(hostMemory->Alloc(mem::kPageSize));
                }
            }, std::ref(barrier), hostMemory, std::ref(threadAllocatedRegion.emplace_back()));
        }

        for (auto& t : threads)
        {
            t.join();
        }

        std::set<IHostMemory::MemRegion> allRegions;

        for (auto& regionCollection : threadAllocatedRegion)
        {
            for (IHostMemory::MemRegion& region : regionCollection)
            {
                auto [_, emplaceOk] = allRegions.emplace(std::move(region));
                ASSERT_TRUE(emplaceOk);
            }
        }

        auto iter = allRegions.begin();

        const IHostMemory::MemRegion* prevRegion = &*(iter++);
        for (; iter != allRegions.end(); ++iter)
        {
            ASSERT_TRUE(IHostMemory::MemRegion::IsAdjacent(*prevRegion, *iter));
            ASSERT_TRUE(IHostMemory::MemRegion::IsAdjacent(*iter, *prevRegion));
            prevRegion = &*(iter);
        }
    }

}  // namespace my::test
