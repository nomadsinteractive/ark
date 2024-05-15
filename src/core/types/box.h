#pragma once

#include <memory>

#include "core/base/api.h"
#include "core/inf/duck.h"
#include "core/forwarding.h"
#include "core/types/interfaces.h"
#include "core/types/type.h"

namespace ark {

class ARK_API Box {
public:
    typedef void(*Destructor)(const void*);

    constexpr Box() noexcept = default;
    constexpr Box(std::nullptr_t) noexcept {}
    template<typename T> Box(sp<T> sharedPtr) noexcept
        : Box(sharedPtr ? new SharedPtr<T>(std::move(sharedPtr)) : nullptr) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Box);

    explicit operator bool() const;

//  [[script::bindings::property]]
    TypeId typeId() const;

    template<typename T> const sp<T>& unpack() const {
        return _stub ? _stub->unpack<T>() : sp<T>::null();
    }

    template<typename T> sp<T> as() const {
        if(!_stub)
            return sp<T>::null();

        TypeId typeId = Type<T>::id();
        sp<T> inst = typeId == _stub->typeId() ? _stub->unpack<T>() : _stub->interfaces()->as(*this, typeId).unpack<T>();
        if(!inst) {
            const sp<Duck<T>> duck = _stub->interfaces()->as(*this, Type<Duck<T>>::id()).template unpack<Duck<T>>();
            if(duck)
                duck->to(inst);
        }
        return inst;
    }

    Box toConcrete() const;

    const void* ptr() const;
    const std::shared_ptr<Interfaces>& interfaces() const;

private:
    template<typename T> Box(const sp<T>* sharedPtr)
        : _stub(sharedPtr ? std::make_shared<Stub>(sharedPtr, sharedPtr->get(), Type<T>::id(), sharedPtr->ensureInterfaces(), _shared_ptr_destructor<T>) : nullptr) {
    }
    Box(const void* sharedPtr, const void* instancePtr, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor destructor) noexcept;

    class ARK_API Stub {
    public:
        Stub(const void* instance, const void* ptr, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor destructor);
        ~Stub();

        const void* ptr() const;
        TypeId typeId() const;
        const std::shared_ptr<Interfaces>& interfaces() const;

        template<typename T> const sp<T>& unpack() const {
            DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
            return _shared_ptr ? *reinterpret_cast<const sp<T>*>(_shared_ptr) : sp<T>::null();
        }

    private:
        const void* _shared_ptr;
        const void* _instance_ptr;
        TypeId _type_id;
        std::shared_ptr<Interfaces> _interfaces;

        Destructor _destructor;

        friend class Box;
    };

    template<typename T> static void _shared_ptr_destructor(const void* inst) {
        delete reinterpret_cast<const SharedPtr<T>*>(inst);
    }

private:
    std::shared_ptr<Stub> _stub;

    template<typename T> friend class SharedPtr;
};

}
