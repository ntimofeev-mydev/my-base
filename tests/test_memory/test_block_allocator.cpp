// #my_engine_source_file
#if !defined(__linux__)
    #include "my/memory/block_allocator.h"
    #include "my/threading/barrier.h"

    #include <intrin.h>

namespace my::test
{
    namespace
    {
        struct CustomAlignedObject
        {
            __m128 value[4];

            CustomAlignedObject()
            {
                value[0] = _mm_setzero_ps();
            }
        };
    }  // namespace

    TEST(BlockAllocator, AllocAligned)
    {
        using namespace my_literals;

        auto allocator = CreateBlockAllocator(CreateHostVirtualMemory(mem::kAllocationGranularity, 256_Kb, true), sizeof(CustomAlignedObject), true);

        auto ptr = allocator->Alloc(256);
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(ptr) % alignof(CustomAlignedObject) == 0);

        [[maybe_unused]] auto obj = new(ptr) CustomAlignedObject;
        std::destroy_at(obj);
        allocator->Free(ptr);
    }

    TEST(BlockAllocator, AllocMultiThread)
    {
        using namespace my_literals;

        constexpr size_t kThreadCount = 10;
        constexpr size_t kIterationCount = 250;
        constexpr size_t kAllocationsCount = 100;
        constexpr size_t kAllocationSize = 128;

        AllocatorPtr allocator = CreateBlockAllocator(CreateHostVirtualMemory(4_Mb, 256_Kb, true), kAllocationSize, true);

        std::vector<std::thread> threads;
        threads.reserve(kThreadCount);
        threading::Barrier barrier{kThreadCount};

        for (size_t i = 0; i < kThreadCount; ++i)
        {
            threads.emplace_back([](threading::Barrier& barrier, AllocatorPtr allocator)
            {
                std::vector<void*> pointers;
                pointers.reserve(kAllocationsCount);

                barrier.Enter();

                for (size_t i = 0; i < kIterationCount; ++i)
                {
                    pointers.clear();

                    for (size_t j = 0; j < kAllocationsCount; ++j)
                    {
                        void* const ptr = allocator->Alloc(kAllocationSize);
                        pointers.push_back(ptr);
                    }

                    for (void* const ptr : pointers)
                    {
                        allocator->Free(ptr);
                    }
                }
            }, std::ref(barrier), allocator);
        }

        for (auto& t : threads)
        {
            t.join();
        }
    }
}  // namespace my::test
#endif