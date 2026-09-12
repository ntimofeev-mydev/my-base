// #my_engine_source_file
#include "my/utils/result.h"

namespace my
{
    Result<void>::Result(Result<>&& other) :
        m_error(std::move(other.m_error))
    {
    }

    Result<void>::operator bool() const
    {
        return !IsError();
    }

    bool Result<void>::IsError() const
    {
        return static_cast<bool>(m_error);
    }

    ErrorPtr Result<void>::GetError() const
    {
        MY_DBG_FATAL(IsError(), "Result<void> has no error");

        return m_error;
    }

    bool Result<>::IsSuccess(ErrorPtr* error) const
    {
        if (m_error && error)
        {
            *error = m_error;
            return false;
        }

        return true;
    }

    void Result<void>::Ignore() const noexcept
    {
        MY_DBG_ASSERT(!m_error, "Ignoring Result<> that holds an error:{}", m_error->GetDesc());
    }

}  // namespace my
