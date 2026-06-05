#pragma once

#include <any>
#include <array>
#include <functional>
#include <memory>

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
        : _type_id(Type<T>::id()), _class(sharedPtr.getClass()), _stub_type(StubType::PTR),
          _stub(sharedPtr ? _make_ptr_stub(new SharedPtr<T>(std::move(sharedPtr))) : nullptr) {
    }
    template<typename T> explicit Box(T value) noexcept
        : _type_id(Type<std::remove_cvref_t<T>>::id()), _class(Class::ensureClass<T>()) {
        if constexpr (std::is_enum_v<T>) {
            _stub_type = StubType::ENUM;
            _stub = std::make_shared<std::any>(static_cast<int32_t>(value));
        } else if constexpr (std::is_trivially_copyable_v<T>) {
            _stub_type = StubType::TRIVIAL;
            _stub = std::make_shared<std::any>(_to_trivial_storage(value));
        } else {
            static_assert(is_specialization_v<T, std::function>, "Only Enum, trivial copyable and std::function types are accepted");
            _stub_type = StubType::FUNCTION;
            _stub = std::make_shared<std::any>(std::move(value));
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
        return std::any_cast<const PtrStub&>(*_stub).unpack<T>();
    }

    template<typename T> T toEnum() const {
        _type_check<T>();
        return static_cast<T>(toEnumValue());
    }

    template<typename T> const T& toFunction() const {
        _type_check<T>();
        ASSERT(_stub && _stub_type == StubType::FUNCTION);
        return *std::any_cast<T>(_stub.get());
    }

    template<typename T> bool isType() const {
        return _type_id == Type<T>::id();
    }

    bool isEnum() const;
    bool isFunction() const;

    int32_t toEnumValue() const;

    template<typename T> T toTrivialValue() const {
        ASSERT(_stub && _stub_type == StubType::TRIVIAL);
        const TrivialCopyableStorage& storage = std::any_cast<const TrivialCopyableStorage&>(*_stub);
        return *reinterpret_cast<const T*>(storage.data());
    }


    Box cast(const TypeId typeId) const;

    template<typename T> sp<T> as() const {
        if(!_stub)
            return nullptr;

        const TypeId typeId = Type<T>::id();
        const PtrStub& ptrStub = std::any_cast<const PtrStub&>(*_stub);
        sp<T> inst = typeId == _type_id ? ptrStub.unpack<T>() : _class->cast(*this, typeId).toPtr<T>();
        if(!inst) {
            if(const sp<Duck<T>> duck = cast(Type<Duck<T>>::id()).template toPtr<Duck<T>>())
                duck->to(inst);
        }
        return inst;
    }

private:
    typedef std::array<int32_t, 4> TrivialCopyableStorage;

    Box(TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor) noexcept;

    // Discriminator for the payload currently held in _stub. The isXXX() queries
    // read this enum instead of probing the std::any for its contained type.
    enum class StubType {
        NONE,
        PTR,
        ENUM,
        TRIVIAL,
        FUNCTION
    };

    struct PtrStub {
        PtrStub(const void* sharedPtr, const void* instancePtr, Destructor destructor)
            : _shared_ptr(sharedPtr, std::move(destructor)), _instance_ptr(instancePtr) {
        }

        template<typename T> sp<T> unpack() const {
            return _shared_ptr ? *static_cast<const sp<T>*>(_shared_ptr.get()) : nullptr;
        }

        // Owns the heap-allocated SharedPtr<T> through a type-erased deleter, which keeps
        // PtrStub copyable (std::any requires it) without risking a double free.
        std::shared_ptr<const void> _shared_ptr;
        const void* _instance_ptr;
    };

    template<typename T> static TrivialCopyableStorage _to_trivial_storage(const T value) {
        static_assert(sizeof(T) <= sizeof(TrivialCopyableStorage));
        TrivialCopyableStorage storage = {};
        *reinterpret_cast<T*>(storage.data()) = value;
        return storage;
    }

    template<typename T> static void _shared_ptr_destructor(const void* inst) {
        delete static_cast<const SharedPtr<T>*>(inst);
    }

    template<typename T> void _type_check() const {
        CHECK(_type_id == Type<T>::id(), "Wrong type being unpacked");
    }

    template<typename T> static std::shared_ptr<std::any> _make_ptr_stub(const sp<T>* sharedPtr) {
        return std::make_shared<std::any>(PtrStub(sharedPtr, sharedPtr->get(), _shared_ptr_destructor<T>));
    }

private:
    TypeId _type_id;
    const Class* _class;
    StubType _stub_type = StubType::NONE;

    std::shared_ptr<std::any> _stub;

    template<typename T> friend class SharedPtr;
};

}
