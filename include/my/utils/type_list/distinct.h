// #my_engine_source_file

#pragma once
#include "my/utils/type_list/type_list.h"

namespace my::type_list_detail
{
    template <typename... R>
    consteval TypeList<R...> Distinct(TypeList<>, TypeList<R...> r)
    {
        return r;
    }

    template <typename H, typename... T, typename... R>
    consteval auto Distinct([[maybe_unused]] TypeList<H, T...> t, TypeList<R...> r)
    {
        if constexpr (type_list::FindIndex<H>(r) < 0)
        {
            return Distinct(TypeList<T...>{}, TypeList<R..., H>{});
        }
        else
        {
            return Distinct(TypeList<T...>{}, r);
        }
    }

}  // namespace my::type_list_detail

namespace my::type_list
{
    template <typename TL>
    using Distinct = decltype(type_list_detail::Distinct(TL{}, TypeList{}));

}  // namespace my::type_list
