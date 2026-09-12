// #my_engine_source_file
#pragma once

#include "my/async/scheduler.h"
// #include "my/async/task.h"
#include "my/base/config.h"
#include "my/rtti/ptr.h"
// #include "my/runtime/async_disposable.h"
// #include "my/runtime/disposable.h"

#include <chrono>
#include <optional>

namespace my
{
    struct MY_ABSTRACT_TYPE WorkQueue : async::Scheduler
    {
        MY_INTERFACE(my::WorkQueue, async::Scheduler);

        // virtual async::Task<> waitForWork() = 0;

        virtual void Poll(std::optional<std::chrono::milliseconds> time = std::chrono::milliseconds{0}) = 0;

        virtual void Notify() = 0;

        // virtual void setName(std::string name) = 0;

        // virtual std::string getName() const = 0;
    };

    using WorkQueuePtr = Ptr<WorkQueue>;

    MY_BASE_EXPORT WorkQueuePtr CreateWorkQueue();

}  // namespace my
