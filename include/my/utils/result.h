// #my_engine_source_file

#pragma once

#include "my/base/config.h"
#include "my/diag/assert.h"
#include "my/diag/error.h"
#include "my/memory/allocator.h"
#include "my/utils/scope_guard.h"
#include "my/utils/type_utils.h"

#include <array>
#include <exception>
#include <optional>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace my
{

    template <typename T = void>
    class Result;

    /**
     */
    template <typename T>
    class [[nodiscard]] Result
    {
        static_assert(!std::is_same_v<T, std::exception_ptr>, "std::exception_ptr is not acceptable type for Result<>");
        static_assert(!IsTemplateOf<Result, T>, "Result is not acceptable type for Result<>");

    public:
        using ValueType = T;

        Result()
            requires(std::is_default_constructible_v<T>)
            :
            m_value(std::in_place)
        {
        }

        Result(const Result& other)
            requires(std::is_copy_constructible_v<T>)
            :
            m_error(other.m_error),
            m_value(other.m_value)
        {
        }

        Result(Result&& other)
            requires(std::is_move_constructible_v<T>)
            :
            m_error(std::move(other.m_error)),
            m_value(std::move(other.m_value))
        {
        }

        template <typename U>
        requires(!std::is_same_v<T, U> && std::is_constructible_v<T, const U&>)
        Result(const Result<U>& other)
        {
            if (other.IsError())
            {
                m_error = other.GetError();
            }
            else
            {
                Emplace(*other);
            }
        }

        template <typename U>
        requires(!std::is_same_v<T, U> && std::is_constructible_v<T, U &&>)
        Result(Result<U>&& other)
        {
            if (other.IsError())
            {
                m_error = other.GetError();
            }
            else
            {
                Emplace(*std::move(other));
            }
        }

        template <typename U>
        requires(std::is_constructible_v<T, U &&>)
        Result(U&& value) noexcept
        {
            Emplace(std::forward<U>(value));
        }
        /**
            construct in-place
        */
        template <typename... A>
        requires(std::is_constructible_v<T, A...>)
        Result(A&&... arg) :
            m_value(std::in_place, std::forward<A>(arg)...)
        {
        }

        /**
            construct error
        */
        template <DerivedFromError U>
        Result(ErrorPtrType<U> error) :
            m_error(std::move(error))
        {
        }

        Result& operator=(const Result&) = default;

        Result& operator=(Result&& other) noexcept
            requires(std::is_move_assignable_v<T>)
        {
            m_error = std::move(other.m_error);
            m_value = std::move(other.m_value);

            return *this;
        }

        template <typename U>
        requires(!std::is_same_v<U, T> && std::is_assignable_v<T&, const U&>)
        Result& operator=(const Result<U>& other)
        {
            if (other.IsError())
            {
                m_value.reset();
                m_error = other.GetError();
            }
            else
            {
                Assign(*other);
            }

            return *this;
        }

        /**
            move assign
        */
        template <typename U>
        requires(!std::is_same_v<U, T> && std::is_assignable_v<T&, U &&>)
        Result& operator=(Result<U>&& other)
        {
            if (other.IsError())
            {
                m_value.reset();
                m_error = other.GetError();
            }
            else
            {
                m_error.Reset();
                Assign(std::move(*other));
            }

            return *this;
        }

        template <typename U>
        requires(std::is_assignable_v<T&, U &&>)
        Result& operator=(U&& value)
        {
            m_error.Reset();
            Assign(std::forward<U>(value));

            return *this;
        }

        template <typename U>
        requires(DerivedFromError<U>)
        Result& operator=(ErrorPtrType<U> error)
        {
            MY_DBG_ASSERT(error);

            m_value.reset();
            m_error = std::move(error);
            return *this;
        }

        template <typename... A>
        requires(std::is_constructible_v<T, A...>)
        void Emplace(A&&... args)
        {
            MY_DBG_ASSERT(!m_error);
            m_value.emplace(std::forward<A>(args)...);
        }

        bool IsError() const
        {
            return static_cast<bool>(m_error);
        }

        my::ErrorPtr GetError() const
        {
            MY_DBG_FATAL(IsError(), "Result<T> has no error");
            return m_error;
        }

        void Ignore() const noexcept
        {
            MY_DBG_ASSERT(!m_error, "Ignoring Result<T> that holds an error:{}", m_error->GetDesc());
        }

        const T& operator*() const&
        {
            MY_DBG_ASSERT(m_value, "Result<T> is valueless");
            return *m_value;
        }

        T& operator*() &
        {
            MY_DBG_FATAL(m_value, "Result<T> is valueless: ({})", m_error ? m_error->GetDesc() : "no error");
            return *m_value;
        }

        T&& operator*() &&
        {
            MY_DBG_FATAL(m_value, "Result<T> is valueless: ({})", m_error ? m_error->GetDesc() : "no error");
            return std::move(*m_value);
        }

        const T* operator->() const
        {
            MY_DBG_ASSERT(m_value, "Result<T> is valueless");
            return &(*m_value);
        }

        T* operator->()
        {
            MY_DBG_ASSERT(m_value, "Result<T> is valueless");
            return &(*m_value);
        }

        explicit operator bool() const
        {
            return m_value.has_value();
        }

    private:
        template <typename U>
        void Assign(U&& value)
        {
            static_assert(std::is_assignable_v<T&, U>);
            m_error.Reset();

            if (m_value)
            {
                *m_value = std::forward<U>(value);
            }
            else
            {
                m_value.emplace(std::forward<U>(value));
            }
        }

        ErrorPtr m_error = nullptr;
        std::optional<T> m_value;
    };

    // template<typename T>
    // Result(const T&) -> Result<T>;

    template <typename T>
    Result(T&&) -> Result<std::remove_const_t<std::remove_reference_t<T>>>;

    /**
     */
    template <>
    class MY_BASE_EXPORT [[nodiscard]] Result<void>
    {
    public:
        using ValueType = void;

        Result() = default;
        Result(const Result<>&) = default;
        Result(Result&& other);

        template <typename U>
        requires(DerivedFromError<U>)
        Result(ErrorPtrType<U> error) :
            m_error(std::move(error))
        {
            MY_DBG_ASSERT(m_error);
        }

        Result<>& operator=(const Result&) = default;
        Result<>& operator=(Result&& other) noexcept
        {
            m_error = std::move(other.m_error);
            return *this;
        }

        template <typename U>
        requires(DerivedFromError<U>)
        Result<>& operator=(ErrorPtrType<U> error)
        {
            MY_DBG_ASSERT(error);
            m_error = std::move(error);
            return *this;
        }

        explicit operator bool() const;

        bool IsError() const;

        bool IsSuccess(ErrorPtr* = nullptr) const;

        my::ErrorPtr GetError() const;

        void Ignore() const noexcept;

    private:
        ErrorPtr m_error;
    };

    template <typename T>
    inline constexpr bool IsResult = my::IsTemplateOf<Result, std::decay_t<T>>;

    inline static Result<> kResultSuccess{};

}  // namespace my

#define CheckResult(expr)                                                          \
    do                                                                             \
    {                                                                              \
        decltype(auto) exprResult = (expr);                                        \
        static_assert(::my::IsResult<decltype(exprResult)>, "Expected Result<T>"); \
        if (exprResult.IsError())                                                  \
        {                                                                          \
            return exprResult.GetError();                                          \
        }                                                                          \
    }                                                                              \
    while (false)\
