// #my_engine_source_file

#pragma once
#include "my/base/config.h"
#include "my/debug/debugger.h"
#include "my/utils/typed_flag.h"

#include <fmt/format.h>

#include <source_location>
#include <string_view>

/*
    MY_DBG_ASSERT(c):  debug break, debug log, release = nothing
    MY_ASSERT:  debug break, debug/release log
    MY_FATAL(c): debug/release log, terminate

*/
#define MY_PANIC void(0)

#if !defined(MY_DBG_ASSERT_MODE)
    #ifdef _DEBUG
        #define MY_DBG_ASSERT_MODE 1
    #else
        #define MY_DBG_ASSERT_MODE 0
    #endif
#endif

namespace my::diag {

    enum class AssertionKind
    {
        Default,
        Fatal
    };

    enum class FailureAction
    {
        DebugBreak = FlagValue(0),
        Abort = FlagValue(1)
    };

    MY_DEFINE_TYPED_FLAG(FailureAction)

    constexpr inline bool kForceFail = false;

}  // namespace my::diag

namespace my::diag_detail {

    consteval inline std::string_view MakeFailureMessage()
    {
        return std::string_view{};
    }

    template <typename... Args>
    inline std::string_view MakeFailureMessage(std::string_view message, const Args&... args)
    {
        if constexpr (sizeof...(Args) == 0)
        {
            return message;
        }
        else
        {
            constexpr size_t kFailureMessageLenMax = 512;
            static thread_local char buffer[kFailureMessageLenMax];
            // std::format_to_n_result result = std::format_to_n(buffer, sizeof(buffer), message, args ...);

            auto end = fmt::vformat_to(buffer, message, fmt::make_format_args(args...));
            //*end = 0;
            return std::string_view{buffer, end};
            // return std::string_view {buffer, result.out};
            // std::string formattedMessage = std::vformat_n_t(std::string_view{message}, std::make_format_args(formatArgs...));
            // return formattedMessage;
            // return std::format(message, makeFormattableArgs<Args>(formatArgs)...);
        }
    }

    MY_BASE_EXPORT diag::FailureActionFlag RaiseFailure(diag::AssertionKind kind, bool logOnly, std::source_location source, std::string_view condition, std::string_view message);

    MY_BASE_EXPORT void SetAutoLogFailure(bool logAssertFailure);

}  // namespace my::diag_detail

#define MY_ASSERT_IMPL(kind, cond, logOnly, ...)                                                                                                                            \
    do                                                                                                                                                                      \
    {                                                                                                                                                                       \
        if (!(cond)) [[unlikely]]                                                                                                                                           \
        {                                                                                                                                                                   \
            const auto action = ::my::diag_detail::RaiseFailure(kind, logOnly, std::source_location::current(), #cond, ::my::diag_detail::MakeFailureMessage(__VA_ARGS__)); \
            if ((action &&::my::diag::FailureAction::DebugBreak) && ::my::debug::IsRunningUnderDebugger())                                                                 \
            {                                                                                                                                                               \
                MY_DEBUG_BREAK;                                                                                                                                             \
            }                                                                                                                                                               \
            if (action &&::my::diag::FailureAction::Abort)                                                                                                               \
            {                                                                                                                                                               \
                MY_PANIC;                                                                                                                                                   \
            }                                                                                                                                                               \
        }                                                                                                                                                                   \
    }                                                                                                                                                                       \
    while (false)

#if MY_DBG_ASSERT_MODE
    #define MY_DBG_ASSERT(cond, ...) MY_ASSERT_IMPL(::my::diag::AssertionKind::Default, cond, false, ##__VA_ARGS__)
    #define MY_ASSERT(cond, ...) MY_ASSERT_IMPL(::my::diag::AssertionKind::Default, cond, false, ##__VA_ARGS__)
    #define MY_DBG_FATAL(cond, ...) MY_ASSERT_IMPL(::my::diag::AssertionKind::Fatal, cond, false, ##__VA_ARGS__)

#else
    #define MY_DBG_ASSERT(cond, ...) void(0)
    #define MY_ASSERT(cond, ...) MY_ASSERT_IMPL(::my::diag::AssertionKind::Default, cond, true, ##__VA_ARGS__)
    #define MY_DBG_FATAL(cond, ...) void(0)
#endif

#define MY_FATAL(cond, ...) MY_ASSERT_IMPL(::my::diag::AssertionKind::Fatal, cond, false, ##__VA_ARGS__)
