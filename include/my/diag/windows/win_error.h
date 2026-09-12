// #my_engine_source_file
#pragma once
#include "my/base/config.h"
#include "my/diag/error.h"

namespace my::diag
{
    /**
     */
    MY_BASE_EXPORT unsigned GetAndResetLastWinError();

    /**
     */
    MY_BASE_EXPORT std::wstring GetWinErrorMessageW(unsigned code);

    /**
     */
    MY_BASE_EXPORT std::string GetWinErrorMessageA(unsigned code);

    /**
     */
    class MY_BASE_EXPORT WinError : public DefaultError<>
    {
        MY_ERROR_CLASS(my::diag::WinError, DefaultError<>);

    public:
        WinError(const std::source_location& sourceLoc, unsigned errorCode = GetAndResetLastWinError());

        WinError(const std::source_location& sourceLoc, std::string message, unsigned errorCode = GetAndResetLastWinError());

        unsigned GetErrorCode() const;

    private:
        const unsigned m_errorCode;
    };

}  // namespace my::diag
