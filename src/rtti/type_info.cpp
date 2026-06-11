// #my_engine_source_file
#include "my/diag/assert.h"
#include "my/rtti/type_info.h"

#include <shared_mutex>


template <>
struct std::hash<::my::rtti_detail::TypeId>
{
    [[nodiscard]]
    size_t operator()(const ::my::rtti_detail::TypeId& val) const
    {
        return val.typeId;
    }
};

namespace my::rtti_detail
{
    struct TypeEntry
    {
        std::string_view typeName;
    };

    struct TypesRegistry
    {
        std::shared_mutex mutex;
        std::unordered_map<TypeId, TypeEntry> types;

        TypesRegistry() = default;
        TypesRegistry(const TypesRegistry&) = delete;
        TypesRegistry& operator=(const TypesRegistry&) = delete;
    };

    TypesRegistry& GetTypesRegistry()
    {
        static TypesRegistry reg;

        return (reg);
    }

    TypeId RegisterRuntimeTypeInternal(const size_t typeHash, std::string_view typeName)
    {
        MY_DBG_ASSERT(typeHash != 0);
        MY_DBG_ASSERT(!typeName.empty());

        if (typeHash == 0 || typeName.empty())
        {
            return {};
        }

        while (!typeName.empty() && typeName.back() == '\0')
        {
            typeName.remove_suffix(1);
        }

        MY_DBG_ASSERT(!typeName.empty());

        auto& registry = GetTypesRegistry();

        const std::lock_guard lock(registry.mutex);

        [[maybe_unused]] auto [iter, emplaceOk] = registry.types.try_emplace(typeHash, TypeEntry{typeName});
        MY_DBG_ASSERT(emplaceOk || iter->second.typeName == typeName);
        return iter->first;
    }

}  // namespace my::rtti_detail

namespace my::rtti
{

    TypeInfo TypeInfo::FromId(size_t typeHash)
    {
        auto& reg = rtti_detail::GetTypesRegistry();

        const std::shared_lock lock(reg.mutex);

        const auto iter = reg.types.find(rtti_detail::TypeId{typeHash});
        return iter != reg.types.end() ? TypeInfo{typeHash, iter->second.typeName} : TypeInfo{};
    }

    TypeInfo TypeInfo::FromName(const char* name)
    {
        if (!name)
        {
            return {};
        }

        auto& reg = rtti_detail::GetTypesRegistry();

        const std::shared_lock lock(reg.mutex);

        for (const auto [typeId, entry] : reg.types)
        {
            if (entry.typeName == name)
            {
                return TypeInfo{typeId, entry.typeName};
            }
        }

        return {};
    }

}  // namespace my::rtti
