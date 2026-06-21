// #my_engine_source_file
#pragma once
#include "my/base/config.h"
#include "my/memory/allocator.h"
#include "my/memory/host_memory.h"


namespace my
{
    MY_BASE_EXPORT AllocatorPtr CreateBlockAllocator(HostMemoryPtr hostMemory, ByteSize blockSize, bool threadSafe);
}  // namespace my
