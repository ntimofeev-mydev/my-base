// #my_engine_source_file

#pragma once

#include "my/base/platform_defs.h"
#include "my/diag/assert.h"
#include "my/meta/class_base.h"
#include "my/rtti/type_info.h"

#include <type_traits>

#define MY_INTERFACE(TypeName, ...) \
    MY_TYPEID(TypeName);            \
    MY_CLASS_BASE(__VA_ARGS__)

namespace my
{
    /**
     */
    struct MY_ABSTRACT_TYPE IRttiObject
    {
        MY_TYPEID(my::IRttiObject);

        virtual ~IRttiObject() = default;
        virtual bool Is(const rtti::TypeInfo&) const noexcept = 0;
        virtual void* As(const rtti::TypeInfo&) noexcept = 0;
        virtual const void* As(const rtti::TypeInfo&) const noexcept = 0;
        virtual void Release() noexcept = 0;

        IRttiObject& operator=(const IRttiObject&) = default;

        template <typename T>
        T As() const requires std::is_pointer_v<T>
        {
            using Interface = std::remove_pointer_t<T>;
            static_assert(std::is_const_v<Interface>, "Attempt to cast through constant instance. const T must be explicitly specified: use 'As<const T*>'");
            static_assert(rtti::HasTypeInfo<std::remove_const_t<Interface>>, "Casting to type without typeinfo");

            const void* const ptr = this->As(rtti::GetTypeInfo<std::remove_const_t<Interface>>());
            return reinterpret_cast<T>(ptr);
        }

        template <typename T>
        T As() requires std::is_pointer_v<T>
        {
            using Interface = std::remove_const_t<std::remove_pointer_t<T>>;
            static_assert(rtti::HasTypeInfo<Interface>, "Casting to type without typeinfo");

            void* const ptr = this->As(rtti::GetTypeInfo<Interface>());
            return reinterpret_cast<T>(ptr);
        }

        template <typename T>
        T As() const requires std::is_reference_v<T>
        {
            using Interface = std::remove_reference_t<T>;
            static_assert(std::is_const_v<Interface>, "Attempt to cast through constant instance. const T must be explicitly specified: use 'As<const T&>'");

            const void* const ptr = this->As(rtti::GetTypeInfo<std::remove_const_t<Interface>>());
            MY_DBG_ASSERT(ptr);

            return *reinterpret_cast<Interface*>(ptr);
        }

        template <typename T>
        T As() requires std::is_reference_v<T>
        {
            using Interface = std::remove_reference_t<T>;
            void* const ptr = this->As(rtti::GetTypeInfo<std::remove_const_t<Interface>>());
            MY_DBG_ASSERT(ptr);

            return *reinterpret_cast<Interface*>(ptr);
        }

        template <typename T, std::enable_if_t<!std::is_reference_v<T> && !std::is_pointer_v<T>, int> = 0>
        T As() const
        {
            static_assert(rtti::HasTypeInfo<T>, "Casting to type without typeinfo");
            constexpr bool kNotPointerOrReference = !(std::is_reference_v<T> || std::is_pointer_v<T>);
            static_assert(kNotPointerOrReference, "Type for 'As' must be pointer or reference: use 'As<T*>' or 'As<T&>'");

            return reinterpret_cast<T*>(nullptr);
        }

        template <typename T>
        bool Is() const
        {
            static_assert(!(std::is_reference_v<T> || std::is_pointer_v<T> || std::is_const_v<T>), "Invalid requested type");
            static_assert(rtti::HasTypeInfo<T>, "Casting to type without typeinfo");

            if constexpr (rtti::HasTypeInfo<T>)
            {
                return this->Is(rtti::GetTypeInfo<T>());
            }
            else
            {
                return false;
            }
        }
    };

    template <typename Derived>
    concept DerivedFromRttiObject = std::is_base_of_v<IRttiObject, Derived>;
}  // namespace my
