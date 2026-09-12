// #my_engine_source_file
#pragma once

#include "my/base/platform_defs.h"
#include "my/diag/assert.h"
#include "my/rtti/cast.h"
#include "my/rtti/ref_counted.h"

#include <memory>

namespace my::rtti_detail
{

    template <DerivedFromRttiObject T>
    MY_FORCE_INLINE IRefCounted& AsRefCounted(T& instance)
    {
        static_assert(!std::is_const_v<T>);

        if constexpr (std::is_convertible_v<T*, IRefCounted*>)
        {
            // The simplest case can directly static cast to IRefCounted
            return static_cast<IRefCounted&>(instance);
        }
        else if constexpr (std::is_base_of_v<IRefCounted, T>)
        {
            // T can inherit IRefCounted through some bases but with non virtual inheritance to IRefCounted.
            // In this case static_cast to IRefCounted is not applicable, but rtti::StaticCast does.
            IRefCounted* const rc = rtti::StaticCast<IRefCounted*>(&instance);
            MY_DBG_FATAL(rc, "Runtime can not cast to IRefCounted for ({})", rtti::GetTypeInfo<T>().GetTypeName());
            return *rc;
        }
        else
        {
            static_assert(std::is_base_of_v<IRttiObject, T>);

            IRttiObject* const rttiBase = rtti::StaticCast<IRttiObject*>(&instance);
            MY_DBG_FATAL(rttiBase);
            IRefCounted* const rc = rttiBase->As<IRefCounted*>();
            MY_DBG_FATAL(rc, "Runtime can not find IRefCounted for ({})", rtti::GetTypeInfo<T>().GetTypeName());
            return *rc;
        }
    }

    template <typename T>
    MY_FORCE_INLINE IRefCounted* TryGetRefCounted(T& instance)
    {
        static_assert(!std::is_const_v<T>);

        if constexpr (std::is_convertible_v<T*, IRefCounted*>)
        {
            return static_cast<IRefCounted*>(&instance);
        }
        else if constexpr (std::is_base_of_v<IRefCounted, T>)
        {
            return rtti::StaticCast<IRefCounted*>(&instance);
        }
        else
        {
            static_assert(std::is_base_of_v<IRttiObject, T>);

            IRttiObject* const rttiBase = rtti::StaticCast<IRttiObject*>(&instance);
            MY_DBG_FATAL(rttiBase);
            return rttiBase->As<IRefCounted*>();
        }
    }
}  // namespace my::rtti_detail

namespace my::rtti
{
    template <typename T>
    struct TakeOwnership
    {
        T* const ptr;

        TakeOwnership(T* ptrIn) :
            ptr{ptrIn}
        {
        }

        TakeOwnership(const TakeOwnership&) = default;
    };

    template <typename T>
    TakeOwnership(T*) -> TakeOwnership<T>;

}  // namespace my::rtti

namespace my
{
    template <typename = IRttiObject>
    class UniqueRttiPtr;

    template <typename = IRefCounted>
    class Ptr;

    /**
     */
    template <typename T>
    class UniqueRttiPtr
    {
        template <typename>
        friend class UniqueRttiPtr;

        template <typename>
        friend class Ptr;

    public:
        using element_type = T;

        ~UniqueRttiPtr()
        {
            if (m_ptr)
            {
                m_ptr->Release();
            }
        }

        UniqueRttiPtr() = default;

        UniqueRttiPtr(std::nullptr_t)
        {
        }

        explicit UniqueRttiPtr(rtti::TakeOwnership<T> ownership) :
            m_ptr(ownership.ptr)
        {
        }

        UniqueRttiPtr(UniqueRttiPtr<T>&& other) :
            m_ptr(std::exchange(other.m_ptr, nullptr))
        {
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        UniqueRttiPtr(UniqueRttiPtr<U>&& other)
        {
            U* const ptr = std::exchange(other.m_ptr, nullptr);
            MoveAssignMaybeCompatible(ptr);
        }

        UniqueRttiPtr(Ptr<T>&& other);

        template <typename U>
        requires(!std::is_same_v<U, T>)
        UniqueRttiPtr(Ptr<U>&& other);

        UniqueRttiPtr(const UniqueRttiPtr&) = delete;

        UniqueRttiPtr& operator=(const UniqueRttiPtr&) = delete;

        UniqueRttiPtr& operator=(std::nullptr_t)
        {
            if (T* const oldPtr = std::exchange(m_ptr, nullptr))
            {
                oldPtr->Release();
            }

            return *this;
        }

        UniqueRttiPtr& operator=(rtti::TakeOwnership<T> ownership)
        {
            if (ownership.ptr != m_ptr) [[likely]]
            {
                if (T* const oldPtr = std::exchange(m_ptr, ownership.ptr))
                {
                    oldPtr->Release();
                }
            }
            

            return *this;
        }

        UniqueRttiPtr& operator=(UniqueRttiPtr&& other)
        {
            if (this->m_ptr != other.m_ptr)
            { 
                T* const newPtr = std::exchange(other.m_ptr, nullptr);
                if (T* const oldPtr = std::exchange(m_ptr, newPtr))
                {
                    oldPtr->Release();
                }
            }

            return *this;
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        UniqueRttiPtr& operator=(UniqueRttiPtr<U>&& other)
        {
            U* const ptr = std::exchange(other.m_ptr, nullptr);
            MoveAssignMaybeCompatible(ptr);
            return *this;
        }

        UniqueRttiPtr& operator=(Ptr<T>&& other);

        template <typename U>
        requires(!std::is_same_v<U, T>)
        UniqueRttiPtr& operator=(Ptr<U>&& other);

        T* GiveUp()
        {
            return std::exchange(m_ptr, nullptr);
        }

        void Release()
        {
            if (T* const ptr = GiveUp())
            {
                ptr->Release();
            }
        }

        T& operator*() const
        {
            MY_DBG_ASSERT(m_ptr, "UniqueRttiPtr<{}> is not dereferenceable", rtti::GetTypeInfo<T>().GetTypeName());
            return *m_ptr;
        }

        T* operator->() const
        {
            MY_DBG_ASSERT(m_ptr, "UniqueRttiPtr<{}> is not dereferenceable", rtti::GetTypeInfo<T>().GetTypeName());
            return m_ptr;
        }

        explicit operator bool() const
        {
            return m_ptr != nullptr;
        }

        bool operator==(std::nullptr_t) const noexcept
        {
            return m_ptr == nullptr;
        }


    private:
        MY_FORCE_INLINE void MoveAssign(T* newPtr)
        {
            if (m_ptr == newPtr) [[unlikely]]
            {
                return;
            }


            if (T* const oldPtr = std::exchange(m_ptr, newPtr))
            {
                oldPtr->Release();
            }
        }

        template <typename U>
        MY_FORCE_INLINE void MoveAssignMaybeCompatible(U* newPtr)
        {
            static_assert(!std::is_same_v<U, T>);

            if constexpr (std::is_convertible_v<U*, T*>)
            {
                MoveAssign(static_cast<T*>(newPtr));
            }
            else
            {
                MY_DBG_ASSERT(newPtr != nullptr, "Assignment for statically incompatible types requires non null value");

                T* const castedPtr = newPtr ? newPtr->template As<T*>() : nullptr;
                MY_DBG_ASSERT(castedPtr, "Can not runtime cast:({}) -> ({})", rtti::GetTypeInfo<U>().GetTypeName(), rtti::GetTypeInfo<T>().GetTypeName());

                // Bad situation,
                if (!castedPtr && newPtr)
                {
                    newPtr->Release();
                }

                MoveAssign(castedPtr);
            }
        }

        T* m_ptr = nullptr;
    };

    template <typename T>
    UniqueRttiPtr(T*) -> UniqueRttiPtr<T>;

    template <typename T>
    UniqueRttiPtr(rtti::TakeOwnership<T>) -> UniqueRttiPtr<T>;

    /**
     */
    template <typename T>
    class Ptr
    {
        template <typename>
        friend class UniqueRttiPtr;

        template <typename>
        friend class Ptr;

    public:
        using type = T;
        using element_type = T;

        ~Ptr()
        {
            if (m_ptr)
            {
                rtti_detail::AsRefCounted(*m_ptr).Release();
            }
        }

        Ptr() = default;

        Ptr(std::nullptr_t)
        {
        }

        Ptr(const Ptr<T>& other) :
            m_ptr{other.m_ptr}
        {
            if (m_ptr != nullptr)
            {
                rtti_detail::AsRefCounted(*m_ptr).AddRef();
            }
        }

        Ptr(Ptr<T>&& other) noexcept :
            m_ptr{other.GiveUp()}
        {
        }

        Ptr(T* ptr) :
            m_ptr{ptr}
        {
            if (m_ptr)
            {
                rtti_detail::AsRefCounted(*m_ptr).AddRef();
            }
        }

        Ptr(rtti::TakeOwnership<T> ownership) :
            m_ptr(ownership.ptr)
        {
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        Ptr(const Ptr<U>& other)

        {
            // static_assert(std::is_convertible_v<U&, T&> || std::is_same_v<T, Com::IRefCountedObject> || std::is_same_v<U, Com::IRefCountedObject>, "Unsafe type cast");
            AssignMaybeCompatible(other.Get());
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        Ptr(Ptr<U>&& other)
        {
            // static_assert(std::is_convertible_v<U&, T&> || std::is_same_v<T, Com::IRefCountedObject> || std::is_same_v<U, Com::IRefCountedObject>, "Unsafe type cast");
            MoveAssignMaybeCompatible(other.GiveUp());
        }

        Ptr(UniqueRttiPtr<T>&& other) :
            m_ptr{other.GiveUp()}
        {
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        Ptr(UniqueRttiPtr<U>&& other)
        {
            MoveAssignMaybeCompatible(other.GiveUp());
        }

        Ptr<T>& operator=(const Ptr<T>& other)
        {
            Assign(other.m_ptr);
            return *this;
        }

        Ptr<T>& operator=(Ptr<T>&& other) noexcept
        {
            MoveAssign(other.GiveUp());
            return *this;
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        Ptr<T>& operator=(const Ptr<U>& other)
        {
            U* const instance = other.Get();
            AssignMaybeCompatible<U>(instance);
            return *this;
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        Ptr<T>& operator=(Ptr<U>&& other)
        {
            MoveAssignMaybeCompatible<U>(other.GiveUp());
            return *this;
        }

        Ptr<T>& operator=(UniqueRttiPtr<T>&& other)
        {
            MoveAssign(other.GiveUp());
            return *this;
        }

        template <typename U>
        requires(!std::is_same_v<U, T>)
        Ptr<T>& operator=(UniqueRttiPtr<U>&& other)
        {
            MoveAssignMaybeCompatible(other.GiveUp());
            return *this;
        }

        T& operator*() const
        {
            MY_DBG_ASSERT(m_ptr, "Ptr<{}> is not dereferenceable", rtti::GetTypeInfo<T>().GetTypeName());
            return *m_ptr;
        }

        T* operator->() const
        {
            MY_DBG_ASSERT(m_ptr, "Ptr<{}> is not dereferenceable", rtti::GetTypeInfo<T>().GetTypeName());
            return m_ptr;
        }

        explicit operator bool() const
        {
            return m_ptr != nullptr;
        }

        bool operator==(std::nullptr_t) const noexcept
        {
            return m_ptr == nullptr;
        }

        bool operator==(const Ptr& other) const noexcept
        {
            return m_ptr == other.m_ptr;
        }

        template <typename U>
        requires(!std::is_same_v<T, U>)
        bool operator==(const Ptr<U>& other) const noexcept
        {
            if (reinterpret_cast<const void*>(m_ptr) == reinterpret_cast<void*>(other.Get()))
            {
                return true;
            }

            return m_ptr != nullptr && other.Get() != nullptr &&
                   &rtti_detail::AsRefCounted(m_ptr) == &rtti_detail::AsRefCounted(*other.Get());
        }

        T* GiveUp()
        {
            return std::exchange(m_ptr, nullptr);
        }

        T* Get() const
        {
            return m_ptr;
        }

        void Reset(T* ptr = nullptr)
        {
            Assign(ptr);
        }

    private:
        /**
            Release current, add ref to new ptr
         */
        void Assign(T* newPtr)
        {
            if (newPtr)
            {
                rtti_detail::AsRefCounted(*newPtr).AddRef();
            }

            if (T* const currentPtr = std::exchange(m_ptr, newPtr); currentPtr)
            {
                rtti_detail::AsRefCounted(*currentPtr).Release();
            }
        }

        /**
            Release current, does nothing for new ptr
         */
        void MoveAssign(T* newPtr)
        {
            if (T* const currentPtr = std::exchange(m_ptr, newPtr); currentPtr)
            {
                rtti_detail::AsRefCounted(*currentPtr).Release();
            }
        }

        /**
            Release current.
            Try to cast new ptr to T, add ref to new ptr
         */
        template <typename U>
        void AssignMaybeCompatible(U* newPtr)
        {
            static_assert(!std::is_same_v<U, T>);

            if constexpr (std::is_convertible_v<U*, T*>)
            {
                Assign(newPtr);
            }
            else
            {
                MY_DBG_ASSERT(newPtr != nullptr, "Assignment for statically incompatible types requires non null value");

                T* const castedPtr = newPtr ? newPtr->template As<T*>() : nullptr;
                    MY_DBG_ASSERT(castedPtr, "Can not runtime cast:({}) -> ({})", rtti::GetTypeInfo<U>().GetTypeName(), rtti::GetTypeInfo<T>().GetTypeName());

                if (!castedPtr && newPtr)
                {
                    newPtr->Release();
                }

                Assign(castedPtr);
            }
        }

        /**
            Release current.
            Try to cast new ptr to T, does nothing for new ptr
         */
        template <typename U>
        void MoveAssignMaybeCompatible(U* newPtr)
        {
            static_assert(!std::is_same_v<U, T>);

            if constexpr (std::is_convertible_v<U*, T*>)
            {
                MoveAssign(newPtr);
            }
            else
            {
                MY_DBG_ASSERT(newPtr != nullptr, "Assignment for statically incompatible types requires non null value");
                T* castedPtr = nullptr;

                if (newPtr)
                {
                    castedPtr = newPtr->template As<T*>();
                    MY_DBG_ASSERT(castedPtr, "Can not runtime cast: ({}) -> ({})",  rtti::GetTypeInfo<U>().GetTypeName(),  rtti::GetTypeInfo<T>().GetTypeName());
                    if (!castedPtr)
                    {
                        rtti_detail::AsRefCounted(*newPtr).Release();
                    }
                }
                
                if (T* const currentPtr = std::exchange(m_ptr, castedPtr); currentPtr)
                {
                    rtti_detail::AsRefCounted(*currentPtr).Release();
                }
            }
        }

        T* m_ptr = nullptr;
    };

    template <typename T>
    Ptr(T*) -> Ptr<T>;

    template <typename T>
    Ptr(rtti::TakeOwnership<T>) -> Ptr<T>;

    template <typename T>
    UniqueRttiPtr<T>::UniqueRttiPtr(Ptr<T>&& other) :
        m_ptr(other.GiveUp())
    {
    }

    template <typename T>
    template <typename U>
    requires(!std::is_same_v<U, T>)
    UniqueRttiPtr<T>::UniqueRttiPtr(Ptr<U>&& other)
    {
        MoveAssignMaybeCompatible(other.GiveUp());
    }

    template <typename T>
    UniqueRttiPtr<T>& UniqueRttiPtr<T>::operator=(Ptr<T>&& other)
    {
        MoveAssign(other.GiveUp());
        return *this;
    }

    template <typename T>
    template <typename U>
    requires(!std::is_same_v<U, T>)
    UniqueRttiPtr<T>& UniqueRttiPtr<T>::operator=(Ptr<U>&& other)
    {
        MoveAssignMaybeCompatible(other.GiveUp());
        return *this;
    }

}  // namespace my
namespace my::rtti
{
    template <typename U, typename T>
    my::Ptr<U> PointerCast(my::Ptr<T>&& ptr)
    {
        return my::Ptr<U>{std::move(ptr)};
    }

    // template <typename U, typename T>
    // std::unique_ptr<U> PointerCast(std::unique_ptr<T>&& ptr)
    // {
    //     return rtti_detail::UniquePtrCastHelper<std::unique_ptr>::cast<U>(std::move(ptr));
    // }

}  // namespace my::rtti
