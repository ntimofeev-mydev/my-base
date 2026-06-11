// #my_engine_source_file
#pragma once
#include "my/base/config.h"
#include "my/utils/string_hash.h"
#include "my/utils/type_tag.h"

// #include <compare>
#include <concepts>
#include <functional>  // std::hash<>
#include <string_view>
#include <type_traits>

namespace my::rtti_detail
{
    struct TypeId
    {
        size_t typeId = 0;

        constexpr TypeId(size_t tid) :
            typeId{tid}
        {
        }

        constexpr TypeId() = default;
        constexpr TypeId(const TypeId&) = default;
        constexpr TypeId& operator=(const TypeId&) = default;
        constexpr explicit operator bool() const
        {
            return typeId > 0;
        }

        constexpr explicit operator size_t() const
        {
            return typeId;
        }

        auto operator<=>(const TypeId&) const noexcept = default;
    };

    template <typename T>
    struct DeclaredTypeId : std::false_type
    {
    };

    template <typename T>
    concept Concept_RttiTypeId = requires {
        { T::MyRtti_TypeId } -> std::same_as<const TypeId&>;
    };

    MY_BASE_EXPORT TypeId RegisterRuntimeTypeInternal(size_t typeHash, std::string_view typeName);

    template <std::size_t N>
    inline TypeId RegisterRuntimeType(const char (&typeName)[N])
    {
        return RegisterRuntimeTypeInternal(strings::ConstHash(typeName), std::string_view{typeName, N});
    }

}  // namespace my::rtti_detail

// clang-format on

#define MY_TYPEID(TypeName)                                                                \
public:                                                                                    \
    static_assert(std::is_trivial_v<::my::TypeTag<TypeName>>, "Check actual type exists"); \
    [[maybe_unused]]                                                                       \
    static inline const ::my::rtti_detail::TypeId MyRtti_TypeId = ::my::rtti_detail::RegisterRuntimeType(#TypeName)

#define MY_DECLARE_TYPEID(TypeName)                                                                                          \
    namespace my::rtti_detail                                                                                                \
    {                                                                                                                        \
        template <>                                                                                                          \
        struct DeclaredTypeId<TypeName> : std::true_type                                                                     \
        {                                                                                                                    \
            static inline const ::my::rtti_detail::TypeId MyRtti_TypeId = ::my::rtti_detail::RegisterRuntimeType(#TypeName); \
        };                                                                                                                   \
    }

// clang-format on

namespace my::rtti
{
    class TypeInfo;

    template <typename T>
    inline constexpr bool HasTypeInfo = rtti_detail::Concept_RttiTypeId<T> || rtti_detail::DeclaredTypeId<T>::value;

    template <typename T>
    concept WithTypeInfo = HasTypeInfo<T>;

    template <typename T>
    concept ClassWithTypeInfo = HasTypeInfo<T> && !std::is_abstract_v<T>;

    template <WithTypeInfo T>
    TypeInfo GetTypeInfo();

    /**
     */
    class [[nodiscard]] MY_BASE_EXPORT TypeInfo
    {
    public:
        static TypeInfo FromId(size_t typeId);
        static TypeInfo FromName(const char* name);

        TypeInfo() = default;
        TypeInfo(const TypeInfo&) = default;
        TypeInfo& operator=(const TypeInfo&) = default;

        constexpr inline size_t GetHashCode() const
        {
            return static_cast<size_t>(m_typeId);
        }

        constexpr std::string_view GetTypeName() const
        {
            return m_typeName;
        }

        constexpr explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_typeId);
        }

        auto operator<=>(const TypeInfo&) const noexcept = default;

    private:
        constexpr TypeInfo(const rtti_detail::TypeId typeId, std::string_view typeName) :
            m_typeId(typeId),
            m_typeName(typeName)
        {
        }

        rtti_detail::TypeId m_typeId;
        std::string_view m_typeName;
    };

    /*
     */
    template <WithTypeInfo T>
    TypeInfo GetTypeInfo()
    {
        using namespace my::rtti_detail;

        const TypeInfo typeInfo = []
        {
            if constexpr (Concept_RttiTypeId<T>)
            {
                return TypeInfo::FromId(static_cast<size_t>(T::MyRtti_TypeId));
            }
            else
            {
                static_assert(DeclaredTypeId<T>::value, "TypeId is not declared");
                return TypeInfo::FromId(static_cast<size_t>(DeclaredTypeId<T>::MyRtti_TypeId));
            }
        }();

        return typeInfo;
    }

}  // namespace my::rtti

template <>
struct std::hash<my::rtti::TypeInfo>
{
    [[nodiscard]]
    size_t operator()(const my::rtti::TypeInfo& val) const
    {
        return val.GetHashCode();
    }
};
