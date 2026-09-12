
#pragma once
#include "my/utils/type_list/type_list.h"

namespace my::type_list_detail
{
    template <typename U, typename... T>
    consteval bool Contains(TypeList<T...>)
    {
        return (std::is_same_v<T, U> || ...);
    }

    template <typename... U, typename... T>
    consteval bool ContainsAll(TypeList<T...> l)
    {
        return (type_list_detail::Contains<U>(l) && ...);
    }
}  // namespace my::type_list_detail

namespace my::type_list
{
    template <typename TL, typename T>
    inline constexpr bool Contains = type_list_detail::Contains<T>(TL{});

}  // namespace my::type_list
