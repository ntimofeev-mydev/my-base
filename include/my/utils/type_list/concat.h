// #my_engine_source_file

#pragma once
#include "my/utils/type_list/type_list.h"

namespace my::type_list_detail
{
    template <typename... T1, typename... T2>
    constexpr auto Concat2List(TypeList<T1...>, TypeList<T2...>)
    {
        return TypeList<T1..., T2...>{};
    }

    consteval TypeList<> Concat()
    {
        return TypeList<>{};
    }

    template <typename H, typename... T>
    consteval auto Concat(H h, T... t) requires(type_list::IsTypeList<H> && (type_list::IsTypeList<T> && ...))
    {
        return Concat2List(h, Concat(t...));
    }

}  // namespace my::type_list_detail

namespace my::type_list
{
    template <typename... TL>
    using Concat = decltype(type_list_detail::Concat(TL{}...));

}  // namespace my::type_list
