// #my_engine_source_file
#include "my/rtti/ptr.h"
#include "my/rtti/ref_counted_class.h"

namespace my::test
{
    namespace
    {
        struct MY_ABSTRACT_TYPE ITestPtrItf1 : IRttiObject
        {
            MY_INTERFACE(my::test::ITestPtrItf1, IRttiObject);
        };

        struct MY_ABSTRACT_TYPE ITestPtrRcItf1 : IRefCounted
        {
            MY_INTERFACE(my::test::ITestPtrRcItf1, IRefCounted);
        };


        class RcObservableDestructor : public ITestPtrItf1, public ITestPtrRcItf1
        {
            MY_REFCOUNTED_CLASS(my::test::RcObservableDestructor, ITestPtrItf1, ITestPtrRcItf1);
        public:
            RcObservableDestructor() = default;
            ~RcObservableDestructor()
            {
                std::cout << "~RcObservableDestructor()\n";
            }
        };

    }


    TEST(RttiPtr, Test1)
    {
        Ptr<> rc1 = rtti::CreateInstance<RcObservableDestructor>();

        UniqueRttiPtr<ITestPtrItf1> p1 = std::move(rc1);
    }



}
