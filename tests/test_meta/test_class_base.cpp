#include "my/meta/class_base.h"

// using namespace br;
// using namespace br::meta;

namespace my::test
{

    using namespace my::meta;

    namespace
    {
        struct SuperBase
        {
        };

        struct Base1 : SuperBase
        {
            MY_CLASS_BASE(SuperBase);
        };

        struct Base2 : SuperBase
        {
            MY_CLASS_BASE(SuperBase);
        };

        class Derived : public Base1,
                        public Base2
        {
            MY_CLASS_BASE(Base1, Base2);

        };
    }  // namespace

    TEST(MetaClassBase, NoBase)
    {
        static_assert(std::is_same_v<meta::ClassAllBase<SuperBase>, TypeList<>>);
    }

    TEST(MetaClassBase, DirectBase)
    {
        static_assert(std::is_same_v<ClassDirectBase<Derived>, TypeList<Base1, Base2>>);
        static_assert(std::is_same_v<ClassDirectBase<Base2>, TypeList<SuperBase>>);
        static_assert(std::is_same_v<ClassDirectBase<SuperBase>, TypeList<>>);
    }

    TEST(MetaClassBase, AllUniqueBase)
    {
        // BE AWARE: types order actually is not specified. Currently it will appear as declared:
        static_assert(std::is_same_v<ClassAllUniqueBase<Derived>, TypeList<Base1, Base2, SuperBase>>);
        static_assert(std::is_same_v<ClassAllUniqueBase<Base2>, TypeList<SuperBase>>);
        static_assert(std::is_same_v<ClassAllUniqueBase<SuperBase>, TypeList<>>);
    }

}  // namespace my::test
