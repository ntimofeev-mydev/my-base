// #my_engine_source_file

#pragma once
#include "my/rtti/rtti_object.h"

namespace my
{
    struct IWeakRef;

    /**
        Ref counted description
    */
    struct MY_ABSTRACT_TYPE IRefCounted : virtual IRttiObject
    {
        MY_INTERFACE(my::IRefCounted, IRttiObject);

        /**
         */
        virtual void AddRef() = 0;

        /**
            Return weak reference
        */
        virtual struct my::IWeakRef* GetWeakRef() = 0;

        /**
         */
        virtual uint32_t GetRefsCount() const = 0;
    };

    /*
     */
    struct MY_ABSTRACT_TYPE IWeakRef
    {
        virtual ~IWeakRef() = default;

        virtual void AddWeakRef() = 0;

        virtual void ReleaseWeak() = 0;

        virtual my::IRefCounted* Lock() = 0;

        virtual bool IsDead() const = 0;
    };

    template<typename Derived>
    concept RefCountedBase = std::is_base_of_v<IRttiObject, Derived>;

}  // namespace my
