#pragma once

#include "my/utils/type_list/type_list.h"

namespace my::type_list_detail
{
    template <template <typename, auto...> class Mapper, typename... T>
    constexpr auto Transform_T(TypeList<T...>)
    {
        return TypeList<typename Mapper<T>::type...>{};
    }

    template <template <typename, auto...> class Mapper, typename... T>
    constexpr auto Transform(TypeList<T...>)
    {
        return TypeList<Mapper<T>...>{};
    }

}  // namespace my::type_list_detail

namespace my::type_list
{
    template <typename TL, template <typename, auto...> class Mapper>
    using Transform = decltype(type_list_detail::Transform<Mapper>(TL{}));

    template <typename TL, template <typename, auto...> class Mapper>
    using Transform_T = decltype(type_list_detail::Transform_T<Mapper>(TL{}));

}  // namespace my::type_list
