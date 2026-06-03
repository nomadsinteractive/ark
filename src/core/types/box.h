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
        : _type_id(0), _class(nullptr) {
    }
    constexpr Box(std::nullptr_t) noexcept
        : _type_id(0), _class(nullptr) {
    }
    template<typename T> explicit Box(sp<T> sharedPtr) noexcept
        : _type_id(Type<T>::id()), _class(sharedPtr.getClass()), _stub(sharedPtr ? _make_ptr_stub(new SharedPtr<T>(std::move(sharedPtr))) : nullptr) {
    }
    template<typename T> explicit Box(const T value) noexcept
        : _type_id(Type<T>::id()), _class(Class::ensureClass<T>()) {
        if constexpr (std::is_enum_v<T>)
            _stub = std::make_shared<_StubVariant>(EnumStub(value));
        else {
            static_assert(std::is_trivial_v<T>, "Only Enum and trivial types are accepted");
            _stub = std::make_shared<_StubVariant>(TrivialStub(value));
        }
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Box);

    explicit operator bool() const;

//  [[script::bindings::property]]
    uintptr_t id() const;
//  [[script::bindings::property]]
    TypeId typeId() const;

    const Class* getClass() const;

    template<typename T> sp<T> toPtr() const {
        if(!_stub)
            return nullptr;
        _type_check<T>();
        return std::get<PtrStub>(*_stub).unpack<T>();
    }

    template<typename T> T toEnum() const {
        _type_check<T>();
        return static_cast<T>(toEnumValue());
    }

    template<typename T> bool isType() const {
        return _type_id == Type<T>::id();
    }

    bool isEnum() const {
        return std::get_if<EnumStub>(_stub.get()) != nullptr;
    }

    template<typename T> T toTrivialValue() const {
        const TrivialStub* stub = std::get_if<TrivialStub>(_stub.get());
        ASSERT(stub);
        return stub->unpack<T>();
    }

    int32_t toEnumValue() const {
        if(!_stub)
            return 0;

        const EnumStub* stub = std::get_if<EnumStub>(_stub.get());
        ASSERT(stub);
        return stub->_value;
    }

    Box cast(const TypeId typeId) const {
        return _class->cast(*this, typeId);
    }

    template<typename T> sp<T> as() const {
        if(!_stub)
            return nullptr;

        const TypeId typeId = Type<T>::id();
        const PtrStub& ptrStub = std::get<PtrStub>(*_stub);
        sp<T> inst = typeId == _type_id ? ptrStub.unpack<T>() : _class->cast(*this, typeId).toPtr<T>();
        if(!inst) {
            if(const sp<Duck<T>> duck = cast(Type<Duck<T>>::id()).template toPtr<Duck<T>>())
                duck->to(inst);
        }
        return inst;
    }

private:
    Box(TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor) noexcept;

    struct PtrStub {
        PtrStub(const void* sharedPtr, const void* instancePtr, Destructor destructor)
            : shared_ptr(sharedPtr), instance_ptr(instancePtr), destructor(std::move(destructor)) {
        }
        PtrStub(PtrStub&& other)
            : shared_ptr(other.shared_ptr), instance_ptr(other.instance_ptr), destructor(other.destructor) {
            other.shared_ptr = nullptr;
        }
        ~PtrStub() {
            if(shared_ptr)
                destructor(shared_ptr);
        }

        template<typename T> sp<T> unpack() const {
            return shared_ptr ? *static_cast<const sp<T>*>(shared_ptr) : nullptr;
        }

        const void* shared_ptr;
        const void* instance_ptr;
        Destructor destructor;
    };

    struct EnumStub {

        template<typename T> T unpack() const {
            return static_cast<T>(_value);
        }

        int32_t _value;
    };

    struct TrivialStub {
        template<typename T> explicit TrivialStub(const T value) {
            static_assert(sizeof(T) <= sizeof(_values));
            *reinterpret_cast<T*>(_values) = value;
        }

        template<typename T> T unpack() const {
            return *reinterpret_cast<const T*>(_values);
        }

        int32_t _values[4] = { 0 };
    };

    template<typename T> static void _shared_ptr_destructor(const void* inst) {
        delete static_cast<const SharedPtr<T>*>(inst);
    }

    template<typename T> void _type_check() const {
        CHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
    }

    typedef std::variant<PtrStub, EnumStub, TrivialStub> _StubVariant;

private:
    template<typename T> static std::shared_ptr<_StubVariant> _make_ptr_stub(const sp<T>* sharedPtr) {
        return sharedPtr ? std::make_shared<_StubVariant>(PtrStub(sharedPtr, sharedPtr->get(), _shared_ptr_destructor<T>)) : nullptr;
    }

private:
    TypeId _type_id;
    const Class* _class;
    std::shared_ptr<_StubVariant> _stub;

    template<typename T> friend class SharedPtr;
};

}
