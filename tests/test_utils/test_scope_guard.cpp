// #my_engine_source_file

// #include "my/diag/error.h"
#include "my/utils/scope_guard.h"

namespace my::test {

    TEST(TestScopeGuard, ScopeLeave)
    {
        bool leave11 = false;
        bool leave21 = false;
        bool leave22 = false;

        scope_leave
        {
            leave11 = true;
        };

        ASSERT_FALSE(leave11);

        {
            scope_leave
            {
                leave21 = true;
            };
            scope_leave
            {
                leave22 = true;
            };

            ASSERT_FALSE(leave21);
            ASSERT_FALSE(leave22);
        }

        ASSERT_TRUE(leave21);
        ASSERT_TRUE(leave22);
    }
#if 0
    TEST(TestScopeGuard, ScopeFailure)
    {
        bool leave = false;
        bool failure = false;
        bool success = false;
        bool neverBeHere = false;

        const auto throwInScope = [&]
        {
            scope_leave
            {
                leave = true;
            };
            scope_leave
            {
                failure = true;
            };
            scope_leave
            {
                success = true;
            };

            throw my::DefaultError{{}, "test_fail"};

            neverBeHere = true;
        };

        try
        {
            throw my::DefaultError{{}, "test_fail"};
        }
        catch(const std::exception&)
        {
        }

        try
        {
            throwInScope();
        }
        catch(const std::exception&)
        {
        }

        ASSERT_TRUE(leave);
        ASSERT_TRUE(failure);
        ASSERT_FALSE(success);
        ASSERT_FALSE(neverBeHere);
    }
#endif

    TEST(TestScopeGuard, ScopeSuccess)
    {
        bool leave = false;
        bool failure = false;
        bool success = false;

        {
            scope_leave
            {
                leave = true;
            };
            scope_fail
            {
                failure = true;
            };
            scope_success
            {
                success = true;
            };
        }

        ASSERT_TRUE(leave);
        ASSERT_FALSE(failure);
        ASSERT_TRUE(success);
    }

    TEST(TestScopeGuard, ScopeNestedException)
    {
    }

}  // namespace my::test
