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

    constexpr Box() noexcept
        : _type_id(0) {
    }
    constexpr Box(std::nullptr_t) noexcept
        : _type_id(0) {
    }
    template<typename T> Box(sp<T> sharedPtr) noexcept
        : _type_id(Type<T>::id()), _stub(sharedPtr ? _make_ptr_stub(new SharedPtr<T>(std::move(sharedPtr))) : nullptr) {
    }
    template<typename T> Box(T enumValue) noexcept
        : _type_id(Type<T>::id()), _stub(_make_enum_stub<T>(enumValue)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Box);

    explicit operator bool() const;

//  [[script::bindings::property]]
    TypeId typeId() const;

    template<typename T> sp<T> toPtr() const {
        if(!_stub)
            return nullptr;
        _type_check<T>();
        return std::get<PtrStub>(*_stub).template unpack<T>();
    }

    template<typename T> T toEnum() const {
        _type_check<T>();
        return static_cast<T>(toInteger());
    }

    int32_t toInteger() const {
        return _stub ? std::get<EnumStub>(*_stub)._value : 0;
    }

    template<typename T> sp<T> as() const {
        if(!_stub)
            return nullptr;

        TypeId typeId = Type<T>::id();
        const PtrStub& ptrStub = std::get<PtrStub>(*_stub);
        sp<T> inst = typeId == _type_id ? ptrStub.unpack<T>() : ptrStub.clazz->cast(*this, typeId).toPtr<T>();
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
        PtrStub(const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor)
            : clazz(clazz), shared_ptr(sharedPtr), instance_ptr(instancePtr), destructor(std::move(destructor)) {
        }
        PtrStub(PtrStub&& other)
            : clazz(other.clazz), shared_ptr(other.shared_ptr), instance_ptr(other.instance_ptr), destructor(std::move(other.destructor)) {
            other.shared_ptr = nullptr;
        }
        ~PtrStub() {
            if(shared_ptr)
                destructor(shared_ptr);
        }

        template<typename T> sp<T> unpack() const {
            return shared_ptr ? *reinterpret_cast<const sp<T>*>(shared_ptr) : nullptr;
        }

        const Class* clazz;
        const void* shared_ptr;
        const void* instance_ptr;
        Destructor destructor;
    };

    struct EnumStub {
        EnumStub(int32_t value)
            : _value(value) {
        }

        template<typename T> T unpack() const {
            return static_cast<T>(_value);
        }

        int32_t _value;
    };

    template<typename T> static void _shared_ptr_destructor(const void* inst) {
        delete reinterpret_cast<const SharedPtr<T>*>(inst);
    }

    template<typename T> void _type_check() const {
        DCHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
    }

    typedef std::variant<PtrStub, EnumStub> _StubVariant;

private:
    template<typename T> static std::shared_ptr<_StubVariant> _make_ptr_stub(const sp<T>* sharedPtr) {
        return sharedPtr ? std::make_shared<_StubVariant>(PtrStub(sharedPtr->getClass(), sharedPtr, sharedPtr->get(), _shared_ptr_destructor<T>)) : nullptr;
    }

    template<typename T> static std::shared_ptr<_StubVariant> _make_enum_stub(T enumValue)  {
        static_assert(std::is_enum_v<T>);
        return std::make_shared<_StubVariant>(EnumStub(enumValue));
    }

private:
    TypeId _type_id;
    std::shared_ptr<_StubVariant> _stub;

    template<typename T> friend class SharedPtr;
};

}
