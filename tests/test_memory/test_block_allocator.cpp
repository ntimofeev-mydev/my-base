// #my_engine_source_file
#if !defined(__linux__)
#include <intrin.h>

#include "my/memory/block_allocator.h"

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
        
        //BlockAllocatorAdataper adapter {createHostVirtualMemory(1_Mb, true), true};


        auto allocator = CreateBlockAllocator(CreateHostVirtualMemory(1_Mb, 256_Kb, true), sizeof(CustomAlignedObject), true);

        auto ptr = allocator->Alloc(sizeof(CustomAlignedObject));
        ASSERT_TRUE(reinterpret_cast<uintptr_t>(ptr) % alignof(CustomAlignedObject) == 0);

        [[maybe_unused]] auto obj = new(ptr) CustomAlignedObject;
        std::destroy_at(obj);
        allocator->Free(ptr);
    }
}  // namespace my::test
#endif