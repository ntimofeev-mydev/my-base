// #my_engine_source_file

// #include "my/rtti/rtti_impl.h"
#include "my/rtti/type_info.h"

namespace my::test
{
    namespace
    {
        struct MyTypeWithTypeId
        {
            MY_TYPEID(MyTypeWithTypeId);
        };

        struct MyTypeWithTypeId2
        {
        };

        struct MyTypeNoTypeId
        {
        };
    }  // namespace

}  // namespace my::test

MY_DECLARE_TYPEID(my::test::MyTypeWithTypeId2)

namespace my::test
{
    namespace
    {

    }  // namespace

    TEST(TypeInfo, HasTypeInfo)
    {
        static_assert(rtti::HasTypeInfo<MyTypeWithTypeId>);
        static_assert(rtti::HasTypeInfo<MyTypeWithTypeId2>);
        static_assert(!rtti::HasTypeInfo<MyTypeNoTypeId>);
    }

    TEST(TypeInfo, GetTypeInfo)
    {
        ASSERT_NE(rtti::GetTypeInfo<MyTypeWithTypeId>().GetHashCode(), 0);
        ASSERT_NE(rtti::GetTypeInfo<MyTypeWithTypeId2>().GetHashCode(), 0);
        ASSERT_NE(rtti::GetTypeInfo<MyTypeWithTypeId2>().GetHashCode(), rtti::GetTypeInfo<MyTypeWithTypeId>().GetHashCode());
    }

    TEST(TypeInfo, FromId)
    {
        const rtti::TypeInfo& typeInfo = rtti::GetTypeInfo<MyTypeWithTypeId>();

        rtti::TypeInfo typeInfo2 = rtti::TypeInfo::FromId(typeInfo.GetHashCode());
        ASSERT_TRUE(typeInfo2);
        ASSERT_EQ(typeInfo2, typeInfo);
    }

    TEST(TypeInfo, FromTypeName)
    {
        const rtti::TypeInfo& typeInfo = rtti::GetTypeInfo<MyTypeWithTypeId>();
        const auto typeName = typeInfo.GetTypeName();

        rtti::TypeInfo typeInfo2 = rtti::TypeInfo::FromName(std::string{typeName}.c_str());
        ASSERT_TRUE(typeInfo2);
        ASSERT_EQ(typeInfo2, typeInfo);
    }

    TEST(TypeInfo, Comparison)
    {
        const auto& typeInfo1 = rtti::GetTypeInfo<MyTypeWithTypeId>();
        const auto& typeInfo2 = rtti::GetTypeInfo<MyTypeWithTypeId2>();

        ASSERT_EQ(typeInfo1, rtti::GetTypeInfo<MyTypeWithTypeId>());
        ASSERT_EQ(typeInfo2, rtti::GetTypeInfo<MyTypeWithTypeId2>());

        ASSERT_NE(typeInfo2, typeInfo1);
    }

    TEST(TypeInfo, AsKey)
    {
        using namespace my::rtti;

        const std::map<TypeInfo, std::string> typeNames = {
            { GetTypeInfo<MyTypeWithTypeId>(), "one"},
            {GetTypeInfo<MyTypeWithTypeId2>(), "two"}
        };

        ASSERT_EQ(std::string{"one"}, typeNames.at(GetTypeInfo<MyTypeWithTypeId>()));
        ASSERT_EQ(std::string{"two"}, typeNames.at(GetTypeInfo<MyTypeWithTypeId2>()));
    }

}  // namespace my::test
