// #my_engine_source_file

#include "my/diag/assert.h"
#include "my/diag/assert_handler.h"
#include "my/utils/scope_guard.h"

namespace my {
    namespace diag {
        namespace {

            static std::atomic<bool> s_autoLogAssertFailure = true;

            AssertHandlerPtr& GetAssertHandlerRef()
            {
                static AssertHandlerPtr s_assertHandler;
                return (s_assertHandler);
            }
        }  // namespace

        void SetAssertHandler(AssertHandlerPtr newHandler, AssertHandlerPtr* prevHandler)
        {
            auto& currentHandlerRef = GetAssertHandlerRef();
            if (prevHandler)
            {
                *prevHandler = std::move(currentHandlerRef);
            }

            currentHandlerRef = std::move(newHandler);
        }

        IAssertHandler* GetCurrentAssertHandler()
        {
            return GetAssertHandlerRef().get();
        }

        AssertHandlerPtr CreateDefaultAssertHandler()
        {
            return nullptr;
        }

    }  // namespace diag

    namespace diag_detail {

        diag::FailureActionFlag RaiseFailure(diag::AssertionKind kind, bool logOnly, std::source_location source, std::string_view condition, std::string_view message)
        {
            using namespace my::diag;

            static thread_local std::atomic<unsigned> raiseFailureCounter = 0;

            if (raiseFailureCounter.fetch_add(1) != 0)
            {
                if (debug::IsRunningUnderDebugger())
                {
                    MY_DEBUG_BREAK;
                }

                MY_PANIC;

                return diag::FailureActionFlag{};
            }

            scope_leave
            {
                raiseFailureCounter.fetch_sub(1);
            };

            if (s_autoLogAssertFailure.load(std::memory_order_relaxed))
            {
                constexpr size_t kFullMessageLenMax = 1024;
                static char buffer[kFullMessageLenMax];

                if (message.empty())
                {
                    // fmt::format_to(buffer, "ASSERT ");
                }
            }

            if (const auto& handler = GetAssertHandlerRef())
            {
                const FailureData data{
                    kind,
                    logOnly,
                    source,
                    condition,
                    message};

                return handler->HandleAssertFailure(data);
            }

            return kind == AssertionKind::Fatal ? (FailureAction::DebugBreak | FailureAction::Abort) : FailureAction::DebugBreak;
        }

        void SetAutoLogFailure(bool logAssertFailure)
        {
            diag::s_autoLogAssertFailure.store(logAssertFailure);
        }
    }  // namespace diag_detail

}  // namespace my
