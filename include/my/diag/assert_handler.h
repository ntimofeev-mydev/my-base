// #my_engine_source_file

#pragma once

#include "my/base/config.h"
#include "my/diag/assert.h"

#include <memory>
#include <source_location>

namespace my::diag {

    /**
     */
    struct FailureData
    {
        /**
         * @brief information about problem.
         * @param condition - possible condition triggered fatal.
         * @param file - file where was triggered fatal.
         * @param line - line where was triggered fatal.
         * @param function - function where was triggered fatal.
         * @param message - additional info about problem.
         */
        const AssertionKind kind;
        const bool logOnly;
        const std::source_location source;
        const std::string_view condition;
        const std::string_view message;

        FailureData(AssertionKind kindP, bool logOnlyP, std::source_location sourceP, std::string_view condP, std::string_view messageP) :
            kind(kindP),
            logOnly(logOnlyP),
            source(sourceP),
            condition(condP),
            message(messageP)
        {
        }

        FailureData(const FailureData&) = default;
    };

    /**
     */
    struct IAssertHandler
    {
        virtual ~IAssertHandler() = default;

        virtual FailureActionFlag HandleAssertFailure(const FailureData&) = 0;
    };

    using AssertHandlerPtr = std::unique_ptr<IAssertHandler>;

    MY_BASE_EXPORT void SetAssertHandler(AssertHandlerPtr newHandler, AssertHandlerPtr* prevHandler = nullptr);

    MY_BASE_EXPORT IAssertHandler* GetCurrentAssertHandler();

    MY_BASE_EXPORT AssertHandlerPtr CreateDefaultAssertHandler();
}  // namespace my::diag
