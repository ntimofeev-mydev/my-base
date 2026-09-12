// #my_engine_source_file

#include "my/utils/type_list/append.h"
#include "my/utils/type_list/concat.h"
#include "my/utils/type_list/contains.h"
#include "my/utils/type_list/distinct.h"
#include "my/utils/type_list/transform.h"

namespace my::test
{
    namespace
    {
        template <typename T>
        struct MakeOptional
        {
            using type = std::optional<T>;
        };
    }  // namespace

    TEST(TestTypeList, Concat)
    {
        static_assert(std::is_same_v<type_list::Concat<TypeList<>>, TypeList<>>);
        static_assert(std::is_same_v<type_list::Concat<TypeList<>, TypeList<>>, TypeList<>>);
        static_assert(std::is_same_v<type_list::Concat<TypeList<int>, TypeList<float>>, TypeList<int, float>>);
        static_assert(std::is_same_v<type_list::Concat<TypeList<unsigned, short>, TypeList<float, double>, TypeList<long>>, TypeList<unsigned, short, float, double, long>>);
    }

    TEST(TestTypeList, Distinct)
    {
        static_assert(std::is_same_v<type_list::Distinct<TypeList<>>, TypeList<>>);
        static_assert(std::is_same_v<type_list::Distinct<TypeList<int>>, TypeList<int>>);
        static_assert(std::is_same_v<type_list::Distinct<TypeList<int, int, int, int>>, TypeList<int>>);
        static_assert(std::is_same_v<type_list::Distinct<TypeList<int, float, int, float>>, TypeList<int, float>>);
    }

    TEST(TestTypeList, Contains)
    {
        using IntsList = TypeList<int, unsigned, short>;

        static_assert(type_list::Contains<IntsList, unsigned>);
        static_assert(type_list::Contains<IntsList, int>);
        static_assert(type_list::Contains<IntsList, short>);
        static_assert(!type_list::Contains<IntsList, float>);
        static_assert(!type_list::Contains<TypeList<>, float>);
    }

    // TEST(TestTypeList, ContainsAll) {
    //
    //	using IntsList = TypeList<short, int, long, unsigned short, unsigned, unsigned long>;
    //
    //	static_assert(type_list::ContainsAll<IntsList, TypeList<int, unsigned>>);
    //	static_assert(type_list::ContainsAll<IntsList, TypeList<>>);
    //	static_assert(!type_list::ContainsAll<IntsList, TypeList<unsigned, double>>);
    //	static_assert(!type_list::ContainsAll<IntsList, TypeList<float>>);
    //
    //	static_assert(!type_list::ContainsAll<TypeList<unsigned>, TypeList<unsigned, float>>);
    //	static_assert(type_list::ContainsAll<TypeList<unsigned, float>, TypeList<unsigned>>);
    //	static_assert(type_list::ContainsAll<TypeList<unsigned>, TypeList<unsigned, unsigned>>);
    // }
    //

    TEST(TestTypeList, Append)
    {
        static_assert(std::is_same_v<type_list::Append<TypeList<>, int>, TypeList<int>>);
        static_assert(std::is_same_v<type_list::Append<TypeList<float>, int>, TypeList<float, int>>);
    }

    TEST(TestTypeList, Prepend)
    {
        static_assert(std::is_same_v<type_list::Prepend<TypeList<>, int>, TypeList<int>>);
        static_assert(std::is_same_v<type_list::Prepend<TypeList<float, double>, int>, TypeList<int, float, double>>);
    }

    TEST(TestTypeList, Transform)
    {
        using InitialTypeList = TypeList<int, unsigned, short>;
        using OptionalsTypeList = TypeList<std::optional<int>, std::optional<unsigned>, std::optional<short>>;
        using TypeList2 = type_list::Transform_T<InitialTypeList, MakeOptional>;
        using TypeList3 = type_list::Transform<InitialTypeList, std::optional>;

        static_assert(std::is_same_v<TypeList2, OptionalsTypeList>);
        static_assert(std::is_same_v<TypeList3, OptionalsTypeList>);
    }

}  // namespace my::test
