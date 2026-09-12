// #my_engine_source_file
#include "my/async/scheduler.h"
#include "my/utils/scope_guard.h"

// #include <chrono>

// using namespace std::chrono;
// using namespace std::chrono_literals;

namespace my::async
{
    namespace
    {
        static SchedulerWeakPtr s_defaultScheduler;
        static thread_local SchedulerWeakPtr s_thisThreadScheduler;
        static thread_local Scheduler::InvokeGuard* s_thisThreadInvokeGuard = nullptr;

        inline Scheduler* GetThisThreadInvokedScheduler()
        {
            return s_thisThreadInvokeGuard ? &s_thisThreadInvokeGuard->executor : nullptr;
        }

    }  // namespace

    Scheduler::InvokeGuard::InvokeGuard(Scheduler& exec) :
        executor(exec),
        threadId(std::this_thread::get_id()),
        prev(s_thisThreadInvokeGuard)
    {
        s_thisThreadInvokeGuard = this;
    }

    Scheduler::InvokeGuard::~InvokeGuard()
    {
        MY_DBG_ASSERT(threadId == std::this_thread::get_id());
        MY_DBG_ASSERT(s_thisThreadInvokeGuard == this);

        s_thisThreadInvokeGuard = s_thisThreadInvokeGuard->prev;
    }

    Scheduler::Invocation::~Invocation() = default;

    Scheduler::Invocation::Invocation(RawCallback cb, void* cbData) :
        m_callback(cb),
        m_callbackData(cbData)
    {
    }

    Scheduler::Invocation::Invocation(Invocation&& other) :
        m_callback(other.m_callback),
        m_callbackData(other.m_callbackData)
    {
        other.Reset();
    }

    Scheduler::Invocation& Scheduler::Invocation::operator=(Invocation&& other)
    {
        m_callback = other.m_callback;
        m_callbackData = other.m_callbackData;

        other.Reset();
        return *this;
    }

    Scheduler::Invocation::operator bool() const
    {
        return m_callback != nullptr;
    }

    void Scheduler::Invocation::operator()()
    {
        MY_DBG_ASSERT(m_callback);
        scope_leave
        {
            Reset();
        };

        if (!m_callback) [[unlikely]]
        {
            return;
        }

        m_callback(m_callbackData);
    }

    void Scheduler::Invocation::Reset()
    {
        m_callback = nullptr;
        m_callbackData = nullptr;
    }

    Scheduler::Invocation Scheduler::Invocation::FromCoroutine(std::coroutine_handle<> coroutine)
    {
        MY_DBG_ASSERT(coroutine);
        if (!coroutine)
        {
            return {};
        }

        return Invocation{[](void* coroAddress) noexcept
        {
            MY_DBG_ASSERT(coroAddress);
            std::coroutine_handle<> coroutine = std::coroutine_handle<>::from_address(coroAddress);
            coroutine();
        }, coroutine.address()};
    }

    SchedulerPtr Scheduler::GetDefault()
    {
        return s_defaultScheduler.Lock();
    }

    SchedulerPtr Scheduler::GetExecuted()
    {
        return GetThisThreadInvokedScheduler();
    }

    SchedulerPtr Scheduler::GetThisThreadScheduler()
    {
        return s_thisThreadScheduler.Lock();
    }

    SchedulerPtr Scheduler::GetCurrent()
    {
        if (auto* const invokedScheduler = GetThisThreadInvokedScheduler())
        {
            return invokedScheduler;
        }

        auto threadScheduler = s_thisThreadScheduler.Lock();
        return threadScheduler ? threadScheduler : s_defaultScheduler.Lock();
    }

    void Scheduler::SetDefault(SchedulerPtr executor)
    {
        s_defaultScheduler = std::move(executor);
    }

    void Scheduler::SetThisThreadScheduler(SchedulerPtr executor)
    {
        s_thisThreadScheduler = std::move(executor);
    }

    // void Scheduler::setSchedulerName(SchedulerPtr executor, std::string_view name);

    // SchedulerPtr Scheduler::findByName(std::string_view name);

    // void Scheduler::Finalize(SchedulerPtr&& executor)
    // {
    //     using namespace std::chrono;

    //     MY_DBG_ASSERT(executor);
    //     if (!executor)
    //     {
    //         return;
    //     }

    //     auto CheckFinalizeTooLong = [time = steady_clock::now(), notified = false]() mutable
    //     {
    //         constexpr milliseconds kShutdownTimeout{5s};

    //         if (steady_clock::now() - time >= kShutdownTimeout && !notified)
    //         {
    //             notified = true;
    //             // core::dumpActiveTasks();
    //         }
    //     };

    //     do
    //     {
    //         executor->WaitAnyActivity();
    //         if (executor->GetRefsCount() == 1)  // the only local reference
    //         {
    //             break;
    //         }

    //         CheckFinalizeTooLong();
    //     }
    //     while (true);
    // }

    void Scheduler::Schedule(Invocation invocation) noexcept
    {
        ScheduleInvocation(std::move(invocation));
    }

    void Scheduler::Schedule(std::coroutine_handle<> coroutine) noexcept
    {
        MY_DBG_ASSERT(coroutine);
        ScheduleInvocation(Invocation::FromCoroutine(std::move(coroutine)));
    }

    void Scheduler::Schedule(RawCallback callback, void* data) noexcept
    {
        ScheduleInvocation(Invocation{callback, data});
    }

    void Scheduler::Invoke([[maybe_unused]] Scheduler& executor, Invocation invocation) noexcept
    {
        MY_DBG_ASSERT(GetThisThreadInvokedScheduler() != nullptr, "Scheduler must be set prior invoke. Use Scheduler::InvokeGuard.");
        MY_DBG_ASSERT(GetThisThreadInvokedScheduler() == &executor, "Invalid executor.");
        MY_DBG_ASSERT(invocation);

        invocation();
    }

    void Scheduler::Invoke([[maybe_unused]] Scheduler& executor, std::span<Invocation> invocations) noexcept
    {
        MY_DBG_ASSERT(GetThisThreadInvokedScheduler() != nullptr, "Scheduler must be set prior invoke. Use Scheduler::InvokeGuard.");
        MY_DBG_ASSERT(GetThisThreadInvokedScheduler() == &executor, "Invalid executor.");

        for (auto& invocation : invocations)
        {
            invocation();
        }
    }

}  // namespace my::async
