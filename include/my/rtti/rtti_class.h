// #my_engine_source_file

#pragma once
#include "my/meta/class_base.h"
#include "my/rtti/cast.h"
#include "my/rtti/type_info.h"

#define MY_IMPLEMENT_RTTI_OBJECT                                           \
                                                                           \
public:                                                                    \
    using my::IRttiObject::Is;                                             \
    using my::IRttiObject::As;                                             \
                                                                           \
    bool Is(const my::rtti::TypeInfo& type) const noexcept override        \
    {                                                                      \
        return my::rtti::StaticIs<decltype(*this)>(type);                  \
    }                                                                      \
                                                                           \
    void* As(const my::rtti::TypeInfo& type) noexcept override             \
    {                                                                      \
        return my::rtti::RuntimeCast(*this, type);                         \
    }                                                                      \
                                                                           \
    const void* As(const my::rtti::TypeInfo& type) const noexcept override \
    {                                                                      \
        return my::rtti::RuntimeCast(*this, type);                         \
    }

#define MY_RTTI_CLASS(ClassImpl, ...) \
    MY_TYPEID(ClassImpl);             \
    MY_CLASS_BASE(__VA_ARGS__);       \
                                      \
    MY_IMPLEMENT_RTTI_OBJECT          \
                                      \
    void Release() noexcept override  \
    {                                 \
        delete this;                  \
    }
