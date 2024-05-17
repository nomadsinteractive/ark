#pragma once

#include <memory>
#include <variant>

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
        : _stub(sharedPtr ? _make_ptr_stub(new SharedPtr<T>(std::move(sharedPtr))) : nullptr) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Box);

    explicit operator bool() const;

//  [[script::bindings::property]]
    TypeId typeId() const;

    template<typename T> const sp<T>& unpack() const {
        return _stub ? std::get<PtrStub>(*_stub).template unpack<T>() : sp<T>::null();
    }

    template<typename T> T toEnum() const {
        DCHECK(_stub);
        return _stub ? std::get<EnumStub>(*_stub).template unpack<T>() : static_cast<T>(0);
    }

    template<typename T> sp<T> as() const {
        if(!_stub)
            return sp<T>::null();

        const PtrStub& ptrStub = std::get<PtrStub>(*_stub);
        TypeId typeId = Type<T>::id();
        sp<T> inst = typeId == ptrStub._type_id ? ptrStub.unpack<T>() : ptrStub._interfaces->as(*this, typeId).unpack<T>();
        if(!inst) {
            const sp<Duck<T>> duck = ptrStub._interfaces->as(*this, Type<Duck<T>>::id()).template unpack<Duck<T>>();
            if(duck)
                duck->to(inst);
        }
        return inst;
    }

    Box toConcrete() const;

    const void* ptr() const;
    const std::shared_ptr<Interfaces>& interfaces() const;

private:
    Box(TypeId typeId, const void* sharedPtr, const void* instancePtr, std::shared_ptr<Interfaces> interfaces, Destructor destructor) noexcept;

    struct PtrStub {
        PtrStub(TypeId typeId, const void* sharedPtr, const void* instancePtr, std::shared_ptr<Interfaces> interfaces, Destructor destructor)
            : _type_id(typeId), _shared_ptr(sharedPtr), _instance_ptr(instancePtr), _interfaces(std::move(interfaces)), _destructor(std::move(destructor)) {
        }
        PtrStub(PtrStub&& other)
            : _type_id(other._type_id), _shared_ptr(other._shared_ptr), _instance_ptr(other._instance_ptr), _interfaces(std::move(other._interfaces)), _destructor(std::move(other._destructor)) {
            other._shared_ptr = nullptr;
        }
        ~PtrStub() {
            if(_shared_ptr)
                _destructor(_shared_ptr);
        }

        template<typename T> const sp<T>& unpack() const {
            DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
            return _shared_ptr ? *reinterpret_cast<const sp<T>*>(_shared_ptr) : sp<T>::null();
        }

        TypeId _type_id;
        const void* _shared_ptr;
        const void* _instance_ptr;
        std::shared_ptr<Interfaces> _interfaces;
        Destructor _destructor;
    };

    struct EnumStub {
        EnumStub(TypeId typeId, int32_t value)
            : _type_id(typeId), _value(value) {
        }

        template<typename T> T unpack() const {
            DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
            return static_cast<T>(_value);
        }

        TypeId _type_id;
        int32_t _value;
    };

    template<typename T> static void _shared_ptr_destructor(const void* inst) {
        delete reinterpret_cast<const SharedPtr<T>*>(inst);
    }

    typedef std::variant<PtrStub, EnumStub> _StubVariant;

private:
    template<typename T> std::shared_ptr<_StubVariant> _make_ptr_stub(const sp<T>* sharedPtr)  {
        return sharedPtr ? std::make_shared<_StubVariant>(PtrStub(Type<T>::id(), sharedPtr, sharedPtr->get(), sharedPtr->ensureInterfaces(), _shared_ptr_destructor<T>)) : nullptr;
    }

    template<typename T> std::shared_ptr<_StubVariant> _make_enum_stub(T enumValue)  {
        return std::make_shared<_StubVariant>(EnumStub(Type<T>::id(), enumValue));
    }

private:
    std::shared_ptr<_StubVariant> _stub;

    template<typename T> friend class SharedPtr;
};

}
