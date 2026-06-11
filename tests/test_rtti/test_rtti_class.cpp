// #my_engine_source_file

#include "my/rtti/rtti_class.h"

#include <memory>


using namespace testing;

namespace my::test
{

    namespace
    {
        struct INonRttiBase
        {
        };

        struct NonRtti1 : INonRttiBase
        {
            MY_CLASS_BASE(INonRttiBase);
        };

        struct NonRtti2 : INonRttiBase
        {
            MY_CLASS_BASE(INonRttiBase);
        };

        struct BaseWithRtti1 : virtual IRttiObject
        {
            MY_INTERFACE(my::test::BaseWithRtti1, IRttiObject);
        };

        struct BaseWithRtti2 : virtual IRttiObject
        {
            MY_INTERFACE(my::test::BaseWithRtti2, IRttiObject);
        };

        class MyClass1 : public NonRtti1,
                         public BaseWithRtti1,
                         public BaseWithRtti2,
                         public NonRtti2
        {
            MY_RTTI_CLASS(test::MyClass1, NonRtti1, BaseWithRtti1, /*BaseWithRtti2,*/ NonRtti2);
        };

    }  // namespace

    TEST(RttiClass, CastToPointer)
    {
        std::unique_ptr<MyClass1> instance{new MyClass1};

        EXPECT_NE(instance->As<BaseWithRtti1*>(), nullptr);
        EXPECT_NE(instance->As<const BaseWithRtti1*>(), nullptr);

        // not declared for external usage
        EXPECT_EQ(instance->As<BaseWithRtti2*>(), nullptr);
        EXPECT_EQ(instance->As<const BaseWithRtti2*>(), nullptr);
    }

    TEST(RttiClass, CastToReference)
    {
        std::unique_ptr<MyClass1> instance{new MyClass1};

        auto& base1 = instance->As<BaseWithRtti1&>();
        const auto& base1c = instance->As<const BaseWithRtti1&>();
    }

    TEST(RttiClass, Is)
    {
        std::unique_ptr<MyClass1> instance{new MyClass1};

        EXPECT_TRUE(instance->Is<BaseWithRtti1>());
        EXPECT_FALSE(instance->Is<BaseWithRtti2>());
    }

    TEST(RttiClass, RuntimeAs)
    {
        std::unique_ptr<MyClass1> instance{new MyClass1};

        void* const ptr1 = instance->As(rtti::GetTypeInfo<BaseWithRtti1>());
        EXPECT_NE(ptr1, nullptr);
    }

}  // namespace my::test
