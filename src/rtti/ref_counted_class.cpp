// #my_engine_source_file
#include "my/rtti/ref_counted_class.h"

namespace my::rtti_detail
{
    namespace
    {
        MY_FORCE_INLINE void Increment(std::atomic<uint32_t>& counter)
        {
            [[maybe_unused]] const auto value = counter.fetch_add(1, std::memory_order_release);
            MY_DBG_ASSERT(value > 0);
        }

        MY_FORCE_INLINE uint32_t Decrement(std::atomic<uint32_t>& counter)
        {
            const auto value = counter.fetch_sub(1, std::memory_order_release);
            MY_DBG_ASSERT(value > 0);
            return value;
        }

        MY_FORCE_INLINE bool TryIncrement(std::atomic<uint32_t>& counter)
        {
            uint32_t value = counter.load(std::memory_order_acquire);
            do
            {
                if (value == 0)
                {
                    return false;
                }
            } while (!counter.compare_exchange_weak(value, counter + 1));

            return true;
        }
    }  // namespace

    RttiClassSharedState::RttiClassSharedState(IAllocator* allocator, LockFunc acquire, DestructorFunc destructor, std::byte* allocatedPtr) :
        m_allocator(allocator),
        m_lockFunc(acquire),
        m_destructorFunc(destructor),
        m_allocatedPtr(allocatedPtr)
    {
        MY_DBG_FATAL(m_lockFunc);
        MY_DBG_FATAL(m_destructorFunc);
        MY_DBG_FATAL(m_allocatedPtr);
        MY_DBG_FATAL(m_stateCounter.load(std::memory_order_relaxed) == 1);
        MY_DBG_FATAL(m_instanceCounter.load(std::memory_order_relaxed) == 1);
    }


    void RttiClassSharedState::AddInstanceRef()
    {
        Increment(m_instanceCounter);
        Increment(m_stateCounter);
    }

    void RttiClassSharedState::ReleaseInstanceRef()
    {
        if (Decrement(m_instanceCounter) == 1)
        {
            void* const instancePtr = RttiClassStorage::GetInstancePtr(*this);
            m_destructorFunc(instancePtr);
        }

        ReleaseStorageRef();
    }

    uint32_t RttiClassSharedState::GetInstanceRefsCount() const
    {
        return m_instanceCounter.load(std::memory_order_relaxed);
    }

    IWeakRef* RttiClassSharedState::GetWeakRef()
    {
        Increment(m_stateCounter);
        return this;
    }

    IAllocator* RttiClassSharedState::GetAllocator() const
    {
        return m_allocator;
    }

    void RttiClassSharedState::ReleaseStorageRef()
    {
        if (Decrement(m_stateCounter) == 1)
        {
            MY_DBG_FATAL(m_stateCounter.load(std::memory_order_relaxed) == 0);
            MY_DBG_FATAL(m_instanceCounter.load(std::memory_order_relaxed) == 0);

            IAllocator* const allocator = m_allocator;
            void* const allocatedPtr = m_allocatedPtr;
            std::destroy_at(this);

            if (allocator)
            {
                allocator->Free(allocatedPtr);
            }
        }
    }

    void RttiClassSharedState::AddWeakRef()
    {
        Increment(m_stateCounter);
    }

    void RttiClassSharedState::ReleaseWeak()
    {
        ReleaseStorageRef();
    }

    IRefCounted* RttiClassSharedState::Lock()
    {
        if (TryIncrement(m_instanceCounter))
        {
            Increment(m_stateCounter);
            void* const instancePtr = RttiClassStorage::GetInstancePtr(*this);
            IRefCounted* const instance = m_lockFunc(instancePtr);
            return instance;
        }

        return nullptr;
    }

    bool RttiClassSharedState::IsDead() const
    {
        MY_DBG_FATAL(m_stateCounter.load() > 0);
        return m_instanceCounter.load(std::memory_order_relaxed) == 0;
    }

    void* RttiClassStorage::AllocateStateAndInstance(std::span<std::byte> inplaceMemBlock, IAllocator* allocator, size_t size, size_t alignment, LockFunc acquireFunc, DestructorFunc destructorFunc)
    {
        MY_DBG_ASSERT((!inplaceMemBlock.empty()) != static_cast<bool>(allocator));

        // plus alignment = extra space for cases when alignment must be adjusted by offset
        const size_t storageSize = kSharedStateSize + size + alignment;
        MY_DBG_FATAL(inplaceMemBlock.empty() || storageSize <= inplaceMemBlock.size());

        // Allocator or preallocated memory
        std::byte* const storage = reinterpret_cast<std::byte*>(allocator ? allocator->Alloc(storageSize) : inplaceMemBlock.data());
        MY_DBG_FATAL(storage);
        MY_DBG_FATAL(reinterpret_cast<uintptr_t>(storage) % alignof(SharedState) == 0);

        std::byte* statePtr = storage;
        std::byte* instanceMemPtr = statePtr + kSharedStateSize;

        // need to respect type's alignment:
        // if storage is not properly aligned there is need to offset state and instance pointers:
        if (const uintptr_t alignmentOffset = reinterpret_cast<uintptr_t>(instanceMemPtr) % alignment; alignmentOffset > 0)
        {
            const size_t offsetGap = alignment - alignmentOffset;
            statePtr = statePtr + offsetGap;
            instanceMemPtr = instanceMemPtr + offsetGap;

            MY_DBG_FATAL(reinterpret_cast<uintptr_t>(statePtr) % alignof(SharedState) == 0);
            MY_DBG_FATAL(reinterpret_cast<uintptr_t>(instanceMemPtr) % alignment == 0);
            MY_DBG_FATAL(storageSize >= kSharedStateSize + size + offsetGap);
        }

        MY_DBG_FATAL(reinterpret_cast<uintptr_t>(instanceMemPtr) % alignment == 0, "Invalid address, expected alignment ({})", alignment);

#ifdef DEBUG
        memset(storage, 0, storageSize);
#endif
        [[maybe_unused]]
        auto sharedState = new(statePtr) SharedState(allocator, acquireFunc, destructorFunc, storage);  // -V799

        return instanceMemPtr;
    }

}  // namespace my::rtti_detail
