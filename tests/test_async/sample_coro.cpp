#include "my/memory/host_memory.h"
#include "my/utils/scope_guard.h"

#include <fmt/format.h>

#include <atomic>
#include <coroutine>
#include <future>
#include <iostream>

using namespace my;

namespace std
{
    template <typename T, typename... Args>
    struct coroutine_traits<std::future<T>, Args...>
    {
        struct promise_type
        {
            std::promise<T> p;

            std::future<T> get_return_object()
            {
                return p.get_future();
            }

            std::suspend_never initial_suspend() noexcept
            {
                return {};
            }

            std::suspend_never final_suspend() noexcept
            {
                return {};
            }

            template <typename U>
            requires(std::is_constructible_v<T, U>)
            void return_value(U&& value)
            {
                p.set_value(std::forward<U>(value));
            }

            void unhandled_exception()
            {
                p.set_exception(std::current_exception());
            }

            void* operator new(size_t size)
            {
                std::cout << fmt::format("Allocate ({}) bytes for coro frame\n", size);
                return malloc(size);
            }

            void operator delete(void* ptr, size_t size)
            {
                std::cout << fmt::format("Free ({}) bytes for coro frame\n", size);
                free(ptr);
            }
        };
    };
}  // namespace std

template <typename T>
struct fut_awaiter
{
    std::future<T>& f;

    bool await_ready() const noexcept
    {
        return false;
    }

    template <typename Promise>
    void await_suspend(std::coroutine_handle<Promise> coro) noexcept
    {
        //auto& prom = coro.promise()

        std::cout << "Will suspend\n";
        std::thread{[](auto coro, std::future<int>& fut)
        {
            fut.wait();
            coro();
        }, coro, std::ref(f)}
            .detach();
    }

    T await_resume()
    {
        return f.get();
    }
};

template <typename T>
auto operator co_await(std::future<T>& fut)
{
    return fut_awaiter{fut};
}

std::future<int> calc1(std::future<int> a)
{
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));

    const int r1 = co_await a;

    std::cout << "Will continue coro, ptr = " << reinterpret_cast<uintptr_t>(buffer) << "\n";

    co_return r1;
}


TEST(SampleCoro, Sample1)
{
    std::promise<int> pp;

    std::future<int> f1 = calc1(pp.get_future());

    pp.set_value(77);

    f1.wait();
    std::cout << fmt::format("Coro res = ({})\n", f1.get());
}