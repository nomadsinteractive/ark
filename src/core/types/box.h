#pragma once

#include <memory>
#include <variant>

#include "core/base/api.h"
#include "core/inf/duck.h"
#include "core/forwarding.h"
#include "core/types/class.h"
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
    template<typename T> Box(T enumValue) noexcept
        : _stub(_make_enum_stub<T>(enumValue)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Box);

    explicit operator bool() const;

//  [[script::bindings::property]]
    TypeId typeId() const;

    template<typename T> sp<T> toPtr() const {
        return _stub ? std::get<PtrStub>(*_stub).template unpack<T>() : nullptr;
    }

    template<typename T> T toEnum() const {
        const EnumStub* enumStub = _stub ? std::get_if<EnumStub>(_stub.get()) : nullptr;
        DCHECK(enumStub, "This variant doesn't contain an enum value");
        return enumStub ? enumStub->unpack<T>() : static_cast<T>(0);
    }

    int32_t toInteger() const {
        return _stub ? std::get<EnumStub>(*_stub)._value : 0;
    }

    template<typename T> sp<T> as() const {
        if(!_stub)
            return nullptr;

        const PtrStub& ptrStub = std::get<PtrStub>(*_stub);
        TypeId typeId = Type<T>::id();
        sp<T> inst = typeId == ptrStub.type_id ? ptrStub.unpack<T>() : ptrStub.clazz->cast(*this, typeId).toPtr<T>();
        if(!inst) {
            const sp<Duck<T>> duck = ptrStub.clazz->cast(*this, Type<Duck<T>>::id()).template toPtr<Duck<T>>();
            if(duck)
                duck->to(inst);
        }
        return inst;
    }

    const void* ptr() const;

private:
    Box(TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor) noexcept;

    struct PtrStub {
        PtrStub(TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor)
            : type_id(typeId), clazz(clazz), shared_ptr(sharedPtr), instance_ptr(instancePtr), destructor(std::move(destructor)) {
        }
        PtrStub(PtrStub&& other)
            : type_id(other.type_id), clazz(other.clazz), shared_ptr(other.shared_ptr), instance_ptr(other.instance_ptr), destructor(std::move(other.destructor)) {
            other.shared_ptr = nullptr;
        }
        ~PtrStub() {
            if(shared_ptr)
                destructor(shared_ptr);
        }

        template<typename T> sp<T> unpack() const {
            DCHECK(type_id == Type<T>::id(), "Wrong type being unpacked");
            return shared_ptr ? *reinterpret_cast<const sp<T>*>(shared_ptr) : nullptr;
        }

        TypeId type_id;
        const Class* clazz;
        const void* shared_ptr;
        const void* instance_ptr;
        Destructor destructor;
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
        return sharedPtr ? std::make_shared<_StubVariant>(PtrStub(Type<T>::id(), sharedPtr->getClass(), sharedPtr, sharedPtr->get(), _shared_ptr_destructor<T>)) : nullptr;
    }

    template<typename T> std::shared_ptr<_StubVariant> _make_enum_stub(T enumValue)  {
        static_assert(std::is_enum_v<T>);
        return std::make_shared<_StubVariant>(EnumStub(Type<T>::id(), enumValue));
    }

private:
    std::shared_ptr<_StubVariant> _stub;

    template<typename T> friend class SharedPtr;
};

}
