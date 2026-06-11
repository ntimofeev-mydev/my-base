// #my_engine_source_file

#pragma once

#include "my/utils/type_list/concat.h"
#include "my/utils/type_list/distinct.h"
#include "my/utils/type_utils.h"

#include <type_traits>

namespace my::meta
{
    /**
     */
    template <typename... T>
    struct ReflectClassBase
    {
        using type = TypeList<T...>;
    };

}  // namespace my::meta

namespace my::meta_detail
{
    /**
     */
    template <typename T>
    concept Concept_ReflectClassBase = requires {
        typename T::My_ClassBase;
    } && IsTemplateOf<meta::ReflectClassBase, typename T::My_ClassBase>;

    /**
     */
    template <typename T, bool = Concept_ReflectClassBase<T>>
    struct ClassDirectBase
    {
        using type = TypeList<>;
    };

    template <typename T>
    struct ClassDirectBase<T, true>
    {
        using type = typename T::My_ClassBase::type;
    };

    /**
     */
    template <typename T, typename Base = typename ClassDirectBase<T>::type>
    struct ClassAllBase;

    template <typename T, typename... Base>
    struct ClassAllBase<T, TypeList<Base...>>
    {
        using type = type_list::Concat<TypeList<Base...>, typename ClassAllBase<Base>::type...>;
    };

    template <typename T>
    using ClassAllUniqueBase = type_list::Distinct<typename ClassAllBase<T>::type>;

}  // namespace my::meta_detail

namespace my::meta
{
    template <typename T>
    using ClassDirectBase = typename ::my::meta_detail::ClassDirectBase<T>::type;

    template <typename T>
    using ClassAllBase = typename ::my::meta_detail::ClassAllBase<T>::type;

    template <typename T>
    using ClassAllUniqueBase = ::my::meta_detail::ClassAllUniqueBase<T>;

}  // namespace my::meta

#define MY_CLASS_BASE(...)                                                                              \
private:                                                                                                \
    void MyValidateClassBase__()                                                                        \
    {                                                                                                   \
        using This = std::remove_pointer_t<decltype(this)>;                                             \
                                                                                                        \
        constexpr bool kAllIsBase = []<typename... BaseT__>(my::TypeList<BaseT__...>) consteval -> bool \
        {                                                                                               \
            return (std::is_base_of_v<BaseT__, This> && ...);                                           \
        }(my::TypeList<__VA_ARGS__>{});                                                                 \
                                                                                                        \
        static_assert(kAllIsBase, "Not all specified types are actually base for type");                \
    }                                                                                                   \
                                                                                                        \
public:                                                                                                 \
    using My_ClassBase = ::my::meta::ReflectClassBase<__VA_ARGS__>
