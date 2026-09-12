// #my_engine_source_file

#pragma once
#include "my/base/platform_defs.h"
#include "my/diag/assert.h"
#include "my/meta/class_base.h"
#include "my/rtti/rtti_object.h"
#include "my/utils/type_list/append.h"
#include "my/utils/type_tag.h"

#include <type_traits>


namespace my::rtti_detail
{
    template <typename Target, typename T, typename... Base>
    MY_FORCE_INLINE bool StaticCastHelper(T& instance, Target*& target, TypeList<Base...>)
    {
        if constexpr (std::is_convertible_v<T*, Target*>)
        {
            target = &static_cast<Target&>(instance);
            return true;
        }
        else
        {
            return (StaticCastHelper(static_cast<Base&>(instance), target, meta::ClassDirectBase<Base>{}) || ...);
        }
    }

    template <typename T, typename... Base>
    MY_FORCE_INLINE bool RuntimeCastHelper(T& instance, const rtti::TypeInfo& target_type, void** target, TypeList<Base...>)
    {
        if constexpr (rtti::HasTypeInfo<T>)
        {
            const rtti::TypeInfo instance_type = rtti::GetTypeInfo<T>();
            if (instance_type == target_type)
            {
                *target = static_cast<void*>(&instance);
                return true;
            }
        }

        return (RuntimeCastHelper(static_cast<Base&>(instance), target_type, target, meta::ClassDirectBase<Base>{}) || ...);
    }

    template <typename T, typename... Base>
    consteval bool IsConvertibleHelper(TypeList<Base...>)
    {
        return (std::is_convertible_v<Base*, T*> || ...);
    }

    template <typename... Base>
    constexpr auto StaticIsHelper(const rtti::TypeInfo& type, TypeList<Base...>)
    {
        const auto isType = []<typename T>(const rtti::TypeInfo& t, TypeTag<T>) constexpr -> bool
        {
            if constexpr (rtti::HasTypeInfo<T>)
            {
                return t == rtti::GetTypeInfo<T>();
            }
            else
            {
                return false;
            }
        };

        return (isType(type, TypeTag<Base>{}) || ...);
    };
}  // namespace my::rtti_detail

namespace my::rtti
{
    template <typename Target, typename U>
    MY_FORCE_INLINE std::enable_if_t<std::is_pointer_v<Target>, Target> StaticCast(U* const instance)
    {
        using Type = std::remove_const_t<U>;
        using TargetType = std::remove_const_t<std::remove_pointer_t<Target>>;

        MY_DBG_ASSERT(instance);

        if constexpr (std::is_same_v<TargetType, Type>)
        {
            return instance;
        }
        else
        {
            Target target = nullptr;
            return rtti_detail::StaticCastHelper(const_cast<Type&>(*instance), target, meta::ClassDirectBase<Type>{}) ? target : nullptr;
        }
    }

    template <typename T>
    MY_FORCE_INLINE void* RuntimeCast(T& instance, const TypeInfo& target_type)
    {
        using Type = std::remove_reference_t<std::remove_const_t<T>>;

        if (target_type == GetTypeInfo<IRttiObject>())
        {
            IRttiObject* const base = StaticCast<IRttiObject*>(&instance);
            return static_cast<void*>(base);
        }

        void* target = nullptr;
        return rtti_detail::RuntimeCastHelper(const_cast<Type&>(instance), target_type, &target, meta::ClassDirectBase<Type>{}) ? target : nullptr;
    }

    template <typename T>
    MY_FORCE_INLINE bool StaticIs(const TypeInfo& target_type)
    {
        using Type = std::remove_reference_t<std::remove_const_t<T>>;
        using UniqueBases = type_list::Prepend<meta::ClassAllUniqueBase<Type>, Type>;

        if (target_type == GetTypeInfo<IRttiObject>())
        {
            return rtti_detail::IsConvertibleHelper<IRttiObject>(UniqueBases{});
        }

        return rtti_detail::StaticIsHelper(target_type, UniqueBases{});
    }
}  // namespace my::rtti
