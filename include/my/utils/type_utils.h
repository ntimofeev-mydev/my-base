// #my_engine_source_file

#pragma once
#include <type_traits>

namespace my
{

    namespace utils_detail
    {

        /**
         */
        template <template <typename... X> class TemplatedClass, typename T>
        struct IsTemplateOf
        {
            static std::false_type Helper(...);

            template <typename... U>
            static std::true_type Helper(const TemplatedClass<U...>&);

            constexpr static bool value = decltype(Helper(std::declval<T>()))::value;
        };

        /**
         */
        template <template <typename...> class TemplatedClass>
        struct IsTemplateOf<TemplatedClass, void>
        {
            constexpr static bool value = false;
        };

    }  // namespace utils_detail

    /**
     */
    struct ConstIndex
    {
        static constexpr inline int kNotIndex = -1;

        const int value;

        constexpr ConstIndex(const ConstIndex& idx) :
            value(idx.value)
        {
        }

        constexpr ConstIndex() :
            value(kNotIndex)
        {
        }

        constexpr ConstIndex(int i) :
            value(i)
        {
        }

        constexpr operator size_t() const
        {
            return static_cast<size_t>(value);
        }

        constexpr operator bool() const
        {
            return value >= 0;
        }

        constexpr ConstIndex operator||(ConstIndex other) const
        {
            return value >= 0 ? *this : other;
        }

        ConstIndex& operator=(const ConstIndex&) = delete;
    };

    /**
     * @brief
     * Tell that type is an instantiation of base template class.
     */
    template <template <typename...> class TemplateClass, typename T>
    inline constexpr bool IsTemplateOf = utils_detail::IsTemplateOf<TemplateClass, std::remove_const_t<std::remove_reference_t<T>>>::value;

    template <typename T, template <typename...> class TemplateClass>
    concept TemplateOfConcept = IsTemplateOf<TemplateClass, T>;

    template <typename T>
    std::add_lvalue_reference_t<T> LValueRef();

    template <typename T>
    std::add_rvalue_reference_t<T> RValueRef();

    template <typename T>
    std::add_const_t<std::add_lvalue_reference_t<T>> ConstLValueRef();

    template <typename T, typename... U>
    inline constexpr bool AnyOf = (std::is_same_v<T, U> || ...);

    // In (msvc) std can not using std::aligned_storage<> with align great than alignof(max_align_t)
    template <size_t Size, size_t Align>
    struct AlignedStorage
    {
        alignas(Align) char space[Size];
    };

}  // namespace my
