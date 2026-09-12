// #my_engine_source_file
#pragma once

#include <initializer_list>
#include <type_traits>

namespace my {

    template <typename T>
    requires(std::is_enum_v<T>)
    class TypedFlag
    {
    public:
        using EnumType = T;
        using ValueType = std::underlying_type_t<T>;

        TypedFlag() = default;

        constexpr TypedFlag(T flag) :
            m_value(static_cast<ValueType>(flag))
        {
        }

        template <typename... U>
        requires(sizeof...(U) > 0 && (std::is_same_v<T, U> && ...))
        constexpr TypedFlag(T value, U... values) :
            m_value(static_cast<ValueType>(value) | (static_cast<ValueType>(values) | ...))
        {
        }

        TypedFlag(const TypedFlag&) = default;

        template <typename... U>
        requires((std::is_same_v<T, U> && ...))
        TypedFlag<T>& Set(U... flags)
        {
            static_assert(sizeof...(flags) > 0);
            const ValueType combinedFlag = (static_cast<ValueType>(flags) | ...);
            m_value |= combinedFlag;
            return *this;
        }

        TypedFlag<T>& Set(TypedFlag<T> flags)
        {
            m_value |= flags.m_value;
            return *this;
        }

        template <typename... U>
        requires((std::is_same_v<T, U> && ...))
        TypedFlag<T>& Unset(U... flags)
        {
            static_assert(sizeof...(flags) > 0);

            const ValueType combinedFlag = (static_cast<ValueType>(flags) | ...);

            m_value &= ~combinedFlag;
            return *this;
        }

        TypedFlag<T>& Unset(TypedFlag<T> flags)
        {
            m_value &= ~flags.m_value;
            return *this;
        }

        constexpr bool IsSet(TypedFlag<T> flags) const
        {
            return (m_value & flags.m_value) == flags.m_value;
        }

        constexpr bool AnyIsSet(TypedFlag<T> flags) const
        {
            return (m_value & flags.m_value) != 0;
        }

        template <typename... U>
        requires((std::is_same_v<T, U> && ...))
        constexpr bool Has(U... flags) const
        {
            static_assert(sizeof...(flags) > 0);

            const ValueType combinedFlag = (static_cast<ValueType>(flags) | ...);
            return (m_value & combinedFlag) == combinedFlag;
        }

        template <typename... U>
        requires((std::is_same_v<T, U> && ...))
        constexpr bool AnyIsSet(U... flags) const
        {
            static_assert(sizeof...(flags) > 0);

            const ValueType combinedFlag = (static_cast<ValueType>(flags) | ...);
            return (m_value & combinedFlag) != 0;
        }

        constexpr bool IsEmpty() const
        {
            return m_value == 0;
        }

        void Clear()
        {
            m_value = 0;
        }

        constexpr operator ValueType() const
        {
            return m_value;
        }

        constexpr explicit operator bool() const
        {
            return m_value != 0;
        }

    private:
        ValueType m_value = 0;

        friend constexpr TypedFlag<T> operator|(TypedFlag<T> value, T flag)
        {
            return TypedFlag<T>{value}.Set(flag);
        }

        friend TypedFlag<T> operator|(TypedFlag<T>, TypedFlag<T>)
        {
        }

        friend TypedFlag<T> operator|(TypedFlag<T>, std::initializer_list<T>)
        {
        }

        friend TypedFlag<T>& operator|=(TypedFlag<T>& value, T flag)
        {
            return value.Set(flag);
        }

        friend TypedFlag<T>& operator|=(TypedFlag<T>& value, TypedFlag<T> flag)
        {
            return value.Set(flag);
        }

        friend TypedFlag<T>& operator|=(TypedFlag<T>&, std::initializer_list<T>)
        {
        }

        friend TypedFlag<T> operator+(TypedFlag<T> value, T flag)
        {
            return TypedFlag<T>{value}.Set(flag);
        }

        friend TypedFlag<T>& operator+=(TypedFlag<T>& value, T flag)
        {
            return value.Set(flag);
        }

        friend TypedFlag<T>& operator-=(TypedFlag<T>& value, T flag)
        {
            return value.Unset(flag);
        }

        friend TypedFlag<T>& operator-=(TypedFlag<T>& value, std::initializer_list<T> flags)
        {
            for (T flag : flags)
            {
                value.m_value &= ~static_cast<ValueType>(flag);
            }

            return value;
        }

        friend TypedFlag<T> operator-(TypedFlag<T> value, T flag)
        {
            return TypedFlag<T>{value}.Unset(flag);
        }

        friend bool operator&&(TypedFlag<T> value, T flag)
        {
            return value.IsSet(flag);
        }

        friend bool operator&&(TypedFlag<T> value, TypedFlag<T> flag)
        {
            return value.IsSet(flag);
        }

        friend constexpr bool operator==(TypedFlag<T> value, T flag)
        {
            return value.m_value == static_cast<ValueType>(flag);
        }

        friend constexpr bool operator==(TypedFlag<T> value1, TypedFlag<T> value2)
        {
            return value1.m_value == value2.m_value;
        }
    };

}  // namespace my

#define FlagValue(x) (1 << x)

#define MY_DEFINE_TYPED_FLAG_OPERATORS(TypedFlag)                                        \
    constexpr inline TypedFlag operator|(TypedFlag::EnumType v1, TypedFlag::EnumType v2) \
    {                                                                                    \
        return {v1, v2};                                                                 \
    }

#define MY_DEFINE_TYPED_FLAG(EnumName)                \
    using EnumName##Flag = ::my::TypedFlag<EnumName>; \
                                                      \
    MY_DEFINE_TYPED_FLAG_OPERATORS(EnumName##Flag)
