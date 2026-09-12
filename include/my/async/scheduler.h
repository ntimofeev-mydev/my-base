// #my_engine_source_file
#pragma once
#include "my/base/config.h"
#include "my/diag/assert.h"
#include "my/rtti/ptr.h"
#include "my/rtti/ref_counted.h"


// #include "my/rtti/rtti_object.h"

#include "my/rtti/weak_ptr.h"
// #include "my/utils/functor.h"
// #include "my/utils/preprocessor.h"

#include <coroutine>
#include <span>
#include <thread>
// #include <type_traits>

namespace my::async
{
    /**
     */
    class MY_BASE_EXPORT MY_ABSTRACT_TYPE Scheduler : public virtual IRefCounted
    {
        MY_INTERFACE(my::async::Scheduler, IRefCounted);

    public:
        using RawCallback = void (*)(void* data) noexcept;

        struct MY_BASE_EXPORT InvokeGuard
        {
            InvokeGuard(Scheduler& exec);
            InvokeGuard(const InvokeGuard&) = delete;
            InvokeGuard(InvokeGuard&&) = delete;

            ~InvokeGuard();

            Scheduler& executor;
            const std::thread::id threadId;
            InvokeGuard* const prev = nullptr;
        };

        class MY_BASE_EXPORT Invocation
        {
        public:
            static Invocation FromCoroutine(std::coroutine_handle<> coroutine);

            Invocation() = default;

            Invocation(RawCallback, void* data);

            Invocation(Invocation&&);

            Invocation(const Invocation&) = delete;

            ~Invocation();

            Invocation& operator=(Invocation&&);

            Invocation& operator=(const Invocation&) = delete;

            explicit operator bool() const;

            void operator()();

        private:
            void Reset();

            RawCallback m_callback = nullptr;
            void* m_callbackData = nullptr;
        };

        /**
         */
        static Ptr<Scheduler> GetDefault();

        /**
         */
        static Ptr<Scheduler> GetExecuted();

        /**
         */
        static Ptr<Scheduler> GetThisThreadScheduler();

        /**
         */
        static Ptr<Scheduler> GetCurrent();

        /**
         */
        static void SetDefault(Ptr<Scheduler>);

        /**
         */
        static void SetThisThreadScheduler(Ptr<Scheduler> executor);

        static void SetSchedulerName(Ptr<Scheduler> executor, std::string_view name);

        /**
         */
        static Ptr<Scheduler> FindByName(std::string_view name);

        static void Finalize(Ptr<Scheduler>&& executor);

        /**
         */
        void Schedule(Invocation invocation) noexcept;

        void Schedule(std::coroutine_handle<>) noexcept;

        void Schedule(RawCallback, void* data) noexcept;

        virtual void HasAnyActivity() const noexcept = 0;

//        virtual void WaitAnyActivity() noexcept = 0;

    protected:
        static void Invoke(Scheduler&, Invocation) noexcept;

        static void Invoke(Scheduler&, std::span<Invocation> invocations) noexcept;

        virtual void ScheduleInvocation(Invocation) noexcept = 0;
    };

    using SchedulerPtr = Ptr<Scheduler>;
    using SchedulerWeakPtr = WeakPtr<Scheduler>;

#if 0
    /*
     *
     */
    struct SchedulerAwaiter
    {
        Scheduler executor;

        SchedulerAwaiter(Scheduler exec) :
            executor(std::move(exec))
        {
            MY_DBG_ASSERT(executor, "Scheduler must be specified");
        }

        constexpr bool await_ready() const noexcept
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> continuation) const
        {
            if (executor)
            {
                executor->execute(std::move(continuation));
            }
        }

        constexpr void await_resume() const noexcept
        {
        }
    };

    inline SchedulerAwaiter operator co_await(Scheduler executor)
    {
        return {std::move(executor)};
    }

    inline SchedulerAwaiter operator co_await(SchedulerWeakPtr executorWeakRef)
    {
        auto executor = executorWeakRef.acquire();
        MY_DBG_ASSERT(executor, "Scheduler instance expired");

        return {std::move(executor)};
    }
#endif

}  // namespace my::async

#define ASYNC_SWITCH_EXECUTOR(executorExpression)                         \
    do                                                                    \
    {                                                                     \
        ::my::async::SchedulerPtr executorVar = executorExpression;        \
        MY_DBG_ASSERT(executorVar);                                       \
                                                                          \
        if (my::async::Scheduler::GetCurrent().Get() != executorVar.Get()) \
        {                                                                 \
            co_await executorVar;                                         \
        }                                                                 \
    }                                                                     \
    while (false)\
