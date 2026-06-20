// #my_engine_source_file
#include "my/diag/windows/win_error.h"
#include "my/utils/scope_guard.h"
#include "my/utils/string_conv.h"
#include "my/windows/windows_headers.h"

#include <fmt/format.h>

namespace my::diag
{
    namespace
    {
        // https://msdn.microsoft.com/en-us/library/ms679351(v=VS.85).aspx
        inline DWORD FormatMessageHelper(DWORD messageId, LPWSTR& buffer)
        {
            return FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, messageId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
        }

        inline DWORD FormatMessageHelper(DWORD messageId, LPSTR& buffer)
        {
            return FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, messageId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
        }

        template <typename Char>
        std::basic_string<Char> GetWindowsMessage(DWORD messageId)
        {
            if (messageId == 0)
            {
                return {};
            }

            Char* messageBuffer = nullptr;
            scope_leave
            {
                if (messageBuffer)
                {
                    ::LocalFree(messageBuffer);
                }
            };

            std::basic_string<Char> resultMessage;
            const DWORD length = FormatMessageHelper(messageId, messageBuffer);
            if (length == 0)
            {
                SetLastError(0);
            }
            else if (messageBuffer != nullptr)
            {
                resultMessage.assign(messageBuffer, static_cast<size_t>(length));
            }

            return resultMessage;
        }

        inline std::string MakeWinErrorMessage(unsigned code)
        {
            const std::wstring wcsMessage = GetWindowsMessage<wchar_t>(code);
            return strings::WstringToUtf8(wcsMessage);
        }

        inline std::string MakeWinErrorMessage(unsigned code, std::string_view customMessage)
        {
            const std::wstring wcsMessage = GetWindowsMessage<wchar_t>(code);
            const std::string errorMessage = strings::WstringToUtf8(wcsMessage);
            return fmt::format("{}. code:({}):{}", customMessage, code, errorMessage);
        }
    }  // namespace

    WinError::WinError(const std::source_location& sourceLoc, unsigned errorCode) :
        DefaultError<>(sourceLoc, MakeWinErrorMessage(errorCode)),
        m_errorCode(errorCode)
    {
    }

    WinError::WinError(const std::source_location& sourceLoc, std::string message, unsigned errorCode) :
        DefaultError<>(sourceLoc, MakeWinErrorMessage(errorCode, message)),
        m_errorCode(errorCode)
    {
    }

    unsigned WinError::GetErrorCode() const
    {
        return m_errorCode;
    }

    unsigned GetAndResetLastWinError()
    {
        const DWORD error = ::GetLastError();
        if (error != 0)
        {
            SetLastError(0);
        }

        return error;
    }

    std::wstring GetWinErrorMessageW(unsigned errorCode)
    {
        return GetWindowsMessage<wchar_t>(errorCode);
    }

    std::string GetWinErrorMessageA(unsigned errorCode)
    {
        return GetWindowsMessage<char>(errorCode);
    }
}  // namespace my::diag
