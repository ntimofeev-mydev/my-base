// #my_engine_source_file

#include "my/diag/error.h"

namespace my::test
{
    namespace
    {
        struct MY_ABSTRACT_TYPE ITestError : Error
        {
            MY_INTERFACE(ITestError, Error);

            virtual unsigned GetErrorCode() const = 0;
        };

        class TestError final : public DefaultError<ITestError>
        {
            using ErrorBase = DefaultError<ITestError>;

            MY_ERROR_CLASS(TestError, ErrorBase);

        public:
            TestError(const std::source_location& sourceLoc, unsigned errorCode) :
                ErrorBase(sourceLoc, "errorCode"),
                m_errorCode(errorCode)
            {
            }

            unsigned GetErrorCode() const override
            {
                return m_errorCode;
            }

        private:
            const unsigned m_errorCode;
        };
    }  // namespace

    TEST(TestError, MakeDefaultError)
    {
        const char* ErrorText = "test error";

        auto error = MakeError(ErrorText);
        ASSERT_EQ(std::string{ErrorText}, error->GetDesc());
        ASSERT_TRUE(error->Is<my::Error>());
    }

    TEST(TestError, MakeCustomError)
    {
        const std::string ErrorText = "test error";

        auto error2 = MakeErrorT(TestError)(100);
        ASSERT_EQ("errorCode", error2->GetDesc());
        ASSERT_TRUE(error2->Is<my::Error>());
        ASSERT_TRUE(error2->Is<TestError>());
    }

    TEST(TestError, ErrorIsStdException)
    {
        const std::string ErrorText = "test error";
        auto error = MakeError(ErrorText);

        ASSERT_TRUE(error->Is<std::exception>());

        auto& exception = error->As<const std::exception&>();
        ASSERT_EQ(ErrorText, exception.what());
    }

    TEST(TestError, FormattedMessage)
    {
        auto error = MakeError("Text[{}][{}]", 77, 22);
        ASSERT_EQ(error->GetDesc(), std::string_view{"Text[77][22]"});
    }

}  // namespace my::test
