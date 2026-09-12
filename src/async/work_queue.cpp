// #my_engine_source_file
#include "my/async//work_queue.h"
#include "my/rtti/ref_counted_class.h"

#include <concurrentqueue.h>

#include <array>

using namespace std::chrono;
using namespace std::chrono_literals;

namespace my
{
    namespace
    {
        class WorkQueueImpl final : public WorkQueue
        {
            MY_REFCOUNTED_CLASS(my::WorkQueueImpl, WorkQueue);

        private:
            void HasAnyActivity() const noexcept override;
            void ScheduleInvocation(Invocation) noexcept override;
            void Poll(std::optional<milliseconds> time) override;
            void Notify() override;

            moodycamel::ConcurrentQueue<Invocation> m_invocations;
        };

    }  // namespace

    void WorkQueueImpl::HasAnyActivity() const noexcept
    {
    }

    void WorkQueueImpl::ScheduleInvocation(Invocation invocation) noexcept
    {
        m_invocations.enqueue(std::move(invocation));
    }

    void WorkQueueImpl::Poll([[maybe_unused]] std::optional<milliseconds> time)
    {
        MY_DBG_ASSERT(time && *time == 0ms, "blocking or timeouted call not implemented");

        constexpr size_t kBulkSize = 10;

        std::array<Invocation, kBulkSize> invocations;

        do
        {
            size_t count = m_invocations.try_dequeue_bulk(invocations.data(), invocations.size());
            if (count == 0)
            {
                break;
            }

            auto iter = std::remove_if(invocations.data(), invocations.data() + count, [](const Invocation& i)
            {
                return !static_cast<bool>(i);
            });

            count = static_cast<size_t>(iter - invocations.data());

            const InvokeGuard guard{*this};
            Scheduler::Invoke(*this, std::span{invocations.data(), count});
        }
        while (true);
    }

    void WorkQueueImpl::Notify()
    {
    }

    WorkQueuePtr CreateWorkQueue()
    {
        return rtti::CreateInstance<WorkQueueImpl, WorkQueue>();
    }
}  // namespace my