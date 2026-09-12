// #my_engine_source_file
#pragma once

// #include <concepts>
#include <cstddef>
// #include <thread>
#include "my/diag/assert.h"
// #include "my/memory/mem_base.h"
#include "my/memory/allocator.h"
#include "my/rtti/ptr.h"
#include "my/rtti/rtti_class.h"
#include "my/utils/type_utils.h"

#include <span>
#include <type_traits>

// #include "my/utils/tuple_utility.h"

#if defined(DEBUG)
    #define MY_RC_CLASS_VALIDATE_SHARED_STATE
#endif

namespace my
{
    template <typename T>
    concept RefCountedClassWithImplTag = requires {
        typename T::RcClassImplTag;
    } && std::is_same_v<typename T::RcClassImplTag::Type, T>;

}  // namespace my

namespace my::rtti_detail
{
    template <typename T>
    struct MyRcClassImplTag
    {
        using Type = T;
    };

#ifdef MY_RC_CLASS_VALIDATE_SHARED_STATE
    // "Class Marker" is used to validate memory where shared state expected to be allocated.
    // This is need to be ensure that ref counted class created by any rtti factory function
    // with is used special memory layout to place [shared state][instance].
    // Without that the default implementation (with MY_CLASS) of the ref counted class will not be operable.
    // 6004214524017983822 == ['M', 'Y', '_', 'C', 'L', 'A', 'S', 'S']
    inline constexpr uint64_t kMyClassMarkerValue = 6004214524017983822;
#endif

    class MY_BASE_EXPORT RttiClassSharedState final : public my::IWeakRef
    {
    public:
        using LockFunc = IRefCounted* (*)(void*);
        using DestructorFunc = void (*)(void*);

        RttiClassSharedState(const RttiClassSharedState&) = delete;

        RttiClassSharedState(IAllocator* allocator, LockFunc, DestructorFunc, std::byte* ptr);

        RttiClassSharedState operator=(const RttiClassSharedState&) = delete;

        void AddInstanceRef();

        void ReleaseInstanceRef();

        uint32_t GetInstanceRefsCount() const;

        IWeakRef* GetWeakRef();

        IAllocator* GetAllocator() const;

    private:
        void ReleaseStateRef();

        void AddWeakRef() override;

        void ReleaseWeak() override;

        IRefCounted* Lock() override;

        bool IsDead() const override;

#ifdef MY_RC_CLASS_VALIDATE_SHARED_STATE
    public:
        const uint64_t m_classMarker = kMyClassMarkerValue;

    private:
#endif
        IAllocator* m_allocator;
        LockFunc m_lockFunc;
        DestructorFunc m_destructorFunc;
        std::byte* const m_allocatedPtr;
        std::atomic<uint32_t> m_stateCounter{1u};
        std::atomic<uint32_t> m_instanceCounter{1u};
    };

    /**
     */
    struct MY_BASE_EXPORT RttiClassStorage
    {
        using LockFunc = RttiClassSharedState::LockFunc;
        using DestructorFunc = RttiClassSharedState::DestructorFunc;
        using SharedState = RttiClassSharedState;

        static constexpr size_t kSharedStateSize = sizeof(AlignedStorage<sizeof(SharedState), alignof(SharedState)>);

        template <typename T>
        using ValueStorage = AlignedStorage<sizeof(T), std::max(alignof(SharedState), alignof(T))>;

        // additional alignof(T) = extra space for cases when alignment fixed by offset
        template <typename T>
        static constexpr size_t kInstanceStorageSize = kSharedStateSize + sizeof(ValueStorage<T>) + alignof(T);

        template <typename T>
        MY_FORCE_INLINE static SharedState& GetSharedState(const T& instance)
        {
            const std::byte* const instancePtr = reinterpret_cast<const std::byte*>(&instance);
            std::byte* const statePtr = const_cast<std::byte*>(instancePtr - kSharedStateSize);

            auto& sharedState = *reinterpret_cast<SharedState*>(statePtr);

#ifdef MY_RC_CLASS_VALIDATE_SHARED_STATE
            // check that class instance has valid shared state with expected layout.
            MY_FATAL(sharedState.m_classMarker == kMyClassMarkerValue, "Invalid SharedState. RefCounted class must be created only with rtti instance factory functions");
#endif
            return sharedState;
        }

        static void* AllocateStateAndInstance(std::span<std::byte> inplaceMemBlock, IAllocator* allocator, size_t size, size_t alignment, LockFunc, DestructorFunc);

        MY_FORCE_INLINE static void* GetInstancePtr(SharedState& state)
        {
            return reinterpret_cast<std::byte*>(&state) + kSharedStateSize;
        }

        template <typename Klass, typename... Args>
        static Klass* InstanceFactory(std::span<std::byte> inplaceMemBlock, IAllocator* allocator, Args&&... args)
        {
            static_assert(RefCountedClassWithImplTag<Klass>, "Class expected to be implemented with MY_CLASS/MY_REFCOUNTED_CLASS/MY_IMPLEMENT_REFCOUNTED. Please, check Class declaration");
            static_assert((alignof(Klass) <= alignof(SharedState)) || (alignof(Klass) % alignof(SharedState) == 0), "Unsupported type alignment.");

            MY_DBG_ASSERT((!inplaceMemBlock.empty()) != static_cast<bool>(allocator));

            const auto lock = [](void* instancePtr) -> IRefCounted*
            {
                Klass* const instance = reinterpret_cast<Klass*>(instancePtr);
                IRefCounted* const refCounted = rtti::StaticCast<IRefCounted*>(instance);
                MY_DBG_FATAL(refCounted, "Runtime cast ({}) -> IRefCounted failed", rtti::GetTypeInfo<Klass>().GetTypeName());

                return refCounted;
            };

            const auto destructor = [](void* instancePtr)
            {
                Klass* instance = reinterpret_cast<Klass*>(instancePtr);
                std::destroy_at(instance);
            };

            void* const instanceMemPtr = AllocateStateAndInstance(inplaceMemBlock, allocator, sizeof(Klass), alignof(Klass), lock, destructor);
            MY_FATAL(instanceMemPtr);

            return new(instanceMemPtr) Klass(std::forward<Args>(args)...);
        }
    };

}  // namespace my::rtti_detail

namespace my::rtti
{
    template <typename T>
    inline constexpr size_t kInstanceStorageSize = rtti_detail::RttiClassStorage::template kInstanceStorageSize<T>;

    template <typename T>
    using InplaceStorage = AlignedStorage<kInstanceStorageSize<T>, alignof(T)>;

    template <typename Klass, typename Interface = Klass, typename MemBlock, typename... Args>
    requires(std::is_trivial_v<MemBlock> && std::is_standard_layout_v<MemBlock>)
    Ptr<Interface> CreateInstanceInplace(MemBlock& memBlock, Args&&... args)
    {
        using namespace my::rtti_detail;

        static_assert(sizeof(MemBlock) >= kInstanceStorageSize<Klass>);

        const std::span<std::byte> memSpan{reinterpret_cast<std::byte*>(&memBlock), sizeof(MemBlock)};

        Klass* const instance = RttiClassStorage::InstanceFactory<Klass>(memSpan, nullptr, std::forward<Args>(args)...);
        Interface* const itf = StaticCast<Interface*>(instance);
        MY_DBG_FATAL(itf);
        return TakeOwnership{itf};
    }

    template <typename Klass, typename Interface = Klass, typename... Args>
    Ptr<Interface> CreateInstanceSingleton(Args&&... args)
    {
        static InplaceStorage<Klass> storage;
        return CreateInstanceInplace<Klass, Interface>(storage, std::forward<Args>(args)...);
    }

    template <typename Klass, typename Interface = Klass, typename... Args>
    Ptr<Interface> CreateInstanceWithAllocator(IAllocator* allocator, Args&&... args)
    {
        using namespace my::rtti_detail;

        Klass* const instance = RttiClassStorage::InstanceFactory<Klass>({}, allocator, std::forward<Args>(args)...);
        auto const itf = rtti::StaticCast<Interface*>(instance);
        MY_DBG_FATAL(itf);
        return rtti::TakeOwnership(itf);
    }

    template <typename Klass, typename Interface = Klass, typename... Args>
    Ptr<Interface> CreateInstance(Args&&... args)
    {
        return CreateInstanceWithAllocator<Klass, Interface>(GetCrtAllocatorPtr(), std::forward<Args>(args)...);
    }

}  // namespace my::rtti

#define MY_IMPLEMENT_REFCOUNTED(ClassImpl)                                     \
public:                                                                        \
    using RttiClassStorage = ::my::rtti_detail::RttiClassStorage;              \
    using RcClassImplTag = ::my::rtti_detail::MyRcClassImplTag<ClassImpl>;     \
                                                                               \
    void AddRef() override                                                     \
    {                                                                          \
        RttiClassStorage::GetSharedState(*this).AddInstanceRef();              \
    }                                                                          \
                                                                               \
    void Release() noexcept override                                           \
    {                                                                          \
        RttiClassStorage::GetSharedState(*this).ReleaseInstanceRef();          \
    }                                                                          \
                                                                               \
    ::my::IWeakRef* GetWeakRef() override                                      \
    {                                                                          \
        return RttiClassStorage::GetSharedState(*this).GetWeakRef();           \
    }                                                                          \
                                                                               \
    uint32_t GetRefsCount() const override                                     \
    {                                                                          \
        return RttiClassStorage::GetSharedState(*this).GetInstanceRefsCount(); \
    }                                                                          \
                                                                               \
    const ::my::IAllocator* GetInstanceAllocator() const                       \
    {                                                                          \
        return RttiClassStorage::GetSharedState(*this).GetAllocator();         \
    }                                                                          \
                                                                               \
    friend struct my::rtti_detail::RttiClassStorage

#define MY_REFCOUNTED_CLASS(ClassImpl, ...) \
    MY_TYPEID(ClassImpl);                   \
    MY_CLASS_BASE(__VA_ARGS__);             \
                                            \
    MY_IMPLEMENT_RTTI_OBJECT;               \
    MY_IMPLEMENT_REFCOUNTED(ClassImpl)

#ifdef MY_RC_CLASS_VALIDATE_SHARED_STATE
    #undef MY_RC_CLASS_VALIDATE_SHARED_STATE
#endif
