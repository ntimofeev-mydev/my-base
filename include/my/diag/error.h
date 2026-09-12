// #my_engine_source_file
#pragma once


#include "my/rtti/ptr.h"
#include "my/rtti/ref_counted_class.h"
#include "my/rtti/rtti_object.h"

#include <fmt/format.h>

#include <exception>
#include <source_location>
#include <string>
#include <type_traits>


MY_DECLARE_TYPEID(std::exception)

namespace my
{
    /**
     */
    struct MY_BASE_EXPORT MY_ABSTRACT_TYPE Error : virtual IRefCounted,
                                                   virtual std::exception
    {
        MY_INTERFACE(my::Error, IRttiObject, std::exception);

        virtual ~Error() = default;

        [[nodiscard]] virtual std::source_location GetSource() const = 0;

        [[nodiscard]] virtual std::string GetDesc() const = 0;

        [[nodiscard]] std::string GetFullDesc() const;
    };

    template <typename E>
    using ErrorPtrType = Ptr<E>;

    using ErrorPtr = ErrorPtrType<Error>;

    template <typename T>
    concept DerivedFromError = std::is_base_of_v<Error, T>;


    /**
     */
    template <DerivedFromError T = Error>
    class DefaultError : public T
    {
        MY_REFCOUNTED_CLASS(my::DefaultError<T>, T);

    public:
        DefaultError(const std::source_location& sourceLocation, std::string description) :
            m_sourceLocation{sourceLocation},
            m_description{std::move(description)}
        {
        }

        std::source_location GetSource() const override
        {
            return m_sourceLocation;
        }

        std::string GetDesc() const override
        {
            return m_description;
        }

        const char* what() const noexcept(noexcept(std::declval<std::exception>().what())) override
        {
            return this->m_description.c_str();
        }

    private:
        const std::source_location m_sourceLocation;
        const std::string m_description;
    };

    namespace diag_detail
    {
        template <typename>
        struct IsErrorPtrHelper : std::false_type
        {
        };

        template <typename T>
        struct IsErrorPtrHelper<ErrorPtrType<T>> : std::bool_constant<DerivedFromError<T>>
        {
        };

    }  // namespace kernel_detail

    template <typename T>
    constexpr inline bool IsErrorPtr = diag_detail::IsErrorPtrHelper<T>::value;

    template <DerivedFromError ErrorT>
    struct ErrorFactory
    {
        [[maybe_unused]] const std::source_location sourceLocation;

        ErrorFactory(const std::source_location& sourceLocationParam) :
            sourceLocation{sourceLocationParam}
        {
        }

        template <typename... Args>
        inline auto operator()(Args&&... args)
        {
            using ErrorImplType = ErrorT;
            static_assert(!std::is_abstract_v<ErrorImplType>);

            constexpr bool kCanConstructWithSource = std::is_constructible_v<ErrorT, std::source_location, Args...>;
            constexpr bool kCanConstructWithoutSource = std::is_constructible_v<ErrorT, Args...>;

            static_assert(kCanConstructWithSource || kCanConstructWithoutSource, "Invalid error's constructor arguments");
            static_assert(std::is_convertible_v<ErrorImplType*, ErrorT*>, "Implementation type is not compatible with requested error interface");

            if constexpr (kCanConstructWithSource)
            {
                Ptr<ErrorT> error = rtti::CreateInstance<ErrorImplType, ErrorT>(sourceLocation, std::forward<Args>(args)...);
                return error;
            }
            else
            {
                Ptr<ErrorT> error = rtti::CreateInstance<ErrorImplType, ErrorT>(std::forward<Args>(args)...);
                return error;
            }
        }
    };

    template <DerivedFromError E>
    struct ErrorFactory<DefaultError<E>>
    {
        [[maybe_unused]] const std::source_location sourceLocation;

        ErrorFactory(const std::source_location& sourceLocationParam) :
            sourceLocation{sourceLocationParam}
        {
        }

        template <typename StringViewT, typename... Args>
        requires(std::is_constructible_v<std::string_view, StringViewT>)
        ErrorPtr operator()(const StringViewT description, Args&&... args)
        {
            const std::string_view descSv{description};

            if constexpr (sizeof...(Args) == 0)
            {
                return rtti::CreateInstance<DefaultError<E>, Error>(sourceLocation, std::string{descSv});
            }
            else
            {
                constexpr size_t kErrorDescLenMax = 512;
                static thread_local char buffer[kErrorDescLenMax];

                auto end = fmt::vformat_to(buffer, descSv, fmt::make_format_args(args...));
                const size_t len = end - buffer;
                return rtti::CreateInstance<DefaultError<E>, Error>(sourceLocation, std::string{buffer, len});
            }
        }
    };

}  // namespace my

//#define MY_ABSTRACT_ERROR(ErrorType, ...) MY_INTERFACE(ErrorType, __VA_ARGS__)

#define MY_ERROR_CLASS(ErrorType, ...) MY_REFCOUNTED_CLASS(ErrorType, __VA_ARGS__)

#define MakeErrorT(ErrorType) ::my::ErrorFactory<ErrorType>(std::source_location::current())

#define MakeError ::my::ErrorFactory<::my::DefaultError<>>(std::source_location::current())
