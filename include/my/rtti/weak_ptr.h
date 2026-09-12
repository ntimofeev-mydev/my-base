// #my_engine_source_file
#pragma once

#include "my/diag/assert.h"
#include "my/rtti/ptr.h"

namespace my
{

    template <typename T = IRefCounted>
    class WeakPtr
    {
    public:
        using type = T;

        WeakPtr() = default;

        WeakPtr(const WeakPtr<T>& weakPtr) noexcept:
            m_weakRef(weakPtr.m_weakRef)
        {
            if (m_weakRef)
            {
                m_weakRef->AddWeakRef();
            }
        }

        WeakPtr(WeakPtr<T>&& weakPtr)  noexcept:
            m_weakRef(weakPtr.m_weakRef)
        {
            weakPtr.m_weakRef = nullptr;
        }

        explicit WeakPtr(const Ptr<T>& ptr) noexcept
        {
            if (ptr)
            {
                m_weakRef = rtti_detail::AsRefCounted(*ptr).GetWeakRef();
            }
        }

        ~WeakPtr()
        {
            Reset();
        }

        WeakPtr<T>& operator=(const WeakPtr<T>& other)
        {
            if (this != &other)
            {
                IWeakRef* const newWeakRef = other.m_weakRef;
                if (newWeakRef)
                {
                    newWeakRef->AddWeakRef();
                }

                Reset();
                m_weakRef = newWeakRef;
            }

            return *this;
        }

        WeakPtr<T>& operator=(WeakPtr<T>&& other) noexcept
        {
            if (this != &other)
            {
                Reset();
                m_weakRef = std::exchange(other.m_weakRef, nullptr);
            }

            return *this;
        }

        WeakPtr<T>& operator=(const Ptr<T>& ptr)
        {
            Reset();
            if (ptr)
            {
                m_weakRef = rtti_detail::AsRefCounted(*ptr).GetWeakRef();
            }

            return *this;
        }

        Ptr<T> Lock() const
        {
            IRefCounted* const ptr = (m_weakRef != nullptr) ? m_weakRef->Lock() : nullptr;
            if (!ptr)
            {
                return {};
            }

            T* const targetInstance = ptr->As<T*>();
            MY_DBG_ASSERT(targetInstance, "RefCounted object acquired through weak reference, but instance doesn't provide target interface");
            return rtti::TakeOwnership{targetInstance};
        }

        explicit operator bool() const
        {
            return m_weakRef != nullptr;
        }

        bool operator==(const WeakPtr<T>& other) const
        {
            return m_weakRef == other.m_weakRef;
        }

        /**
         * @brief
         * Check that referenced instance is not accessible anymore (i.e. it has been destructed).
         * System can guarantee - that died instance never gone to be alive,
         * but in opposed case alive instance can be a dead immediately right after isAlive returns true;
         */
        bool IsDead() const
        {
            return !m_weakRef || m_weakRef->IsDead();
        }

        void Reset()
        {
            if (m_weakRef)
            {
                IWeakRef* weakRef = std::exchange(m_weakRef, nullptr);
                weakRef->ReleaseWeak();
            }
        }

        IWeakRef* Get() const
        {
            return m_weakRef;
        }

        IWeakRef* GiveUp()
        {
            return std::exchange(m_weakRef, nullptr);
        }

    private:
        IWeakRef* m_weakRef = nullptr;
    };
}  // namespace my
