#ifndef ARK_CORE_TYPES_SHARED_PTR_H_
#define ARK_CORE_TYPES_SHARED_PTR_H_

#include <functional>
#include <memory>

#include "core/forwarding.h"
#include "core/inf/duck.h"
#include "core/types/box.h"
#include "core/types/class.h"
#include "core/types/interfaces.h"
#include "core/types/owned_ptr.h"
#include "core/types/type.h"

namespace ark {

template<typename T> class SharedPtr {
public:
    _CONSTEXPR SharedPtr() noexcept
        : _ptr(nullptr), _interfaces(nullptr) {
    }
    _CONSTEXPR SharedPtr(std::nullptr_t null) noexcept
        : _ptr(null), _interfaces(null) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SharedPtr);
    SharedPtr(std::shared_ptr<T> ptr, std::shared_ptr<Interfaces> interfaces) noexcept
        : _ptr(std::move(ptr)), _interfaces(std::move(interfaces)) {
    }

    template<typename U> SharedPtr(const SharedPtr<U>& ptr) noexcept
        : _ptr(std::static_pointer_cast<T>(ptr._ptr)), _interfaces(ptr._interfaces) {
    }
    template<typename U> SharedPtr(SharedPtr<U>&& ptr) noexcept
        : _ptr(std::move(ptr._ptr)), _interfaces(std::move(ptr._interfaces)) {
    }

    typedef T _PtrType;

    static SharedPtr<T> adopt(T* instance) {
        return SharedPtr<T>(instance);
    }

    static SharedPtr<T> borrow(T* instance) {
        return SharedPtr<T>(instance, nullptr, [](T*) {});
    }

    template<typename... Args> static SharedPtr<T> make(Args&&... args) {
        return SharedPtr<T>(new T(std::forward<Args>(args)...));
    }

    static SharedPtr<T>& null() {
        static SharedPtr<T> inst = nullptr;
        return inst;
    }

    bool operator == (const SharedPtr<T>& sp) const {
        return _ptr == sp._ptr;
    }

    bool operator != (const SharedPtr<T>& sp) const {
        return _ptr != sp._ptr;
    }

    bool operator < (const SharedPtr<T>& sp) const {
        return _ptr < sp._ptr;
    }

    explicit operator bool() const {
        return _ptr.get() != nullptr;
    }

    T* operator ->() const {
        return _ptr.get();
    }

    operator T&() const {
        NOT_NULL(_ptr.get());
        return *_ptr.get();
    }

    T& operator *() const {
        NOT_NULL(_ptr.get());
        return *_ptr.get();
    }

    T* get() const {
        return _ptr.get();
    }

    Box pack() const {
        if(!_ptr)
            return Box();
        SharedPtr<T>* copy = new SharedPtr<T>(*this);
        Box box(copy, Type<T>::id(), _interfaces, packedBoxDestructor);
        return box;
    }

    const std::shared_ptr<T>& ptr() const {
        return _ptr;
    }

    const std::shared_ptr<Interfaces>& interfaces() const {
        return _interfaces;
    }

    template<typename U> SharedPtr<U> cast() const {
        return SharedPtr<U>(std::static_pointer_cast<U>(_ptr), _interfaces);
    }

    template<typename U> SharedPtr<T>& absorb(const SharedPtr<U>& beingAbsorbed) {
        if(beingAbsorbed.interfaces() != _interfaces)
            _interfaces->absorb(beingAbsorbed.pack());
        return *this;
    }

    template<typename U> bool is() const {
        return _interfaces ? _interfaces->isInstance<U>() : false;
    }

    template<typename U> SharedPtr<U> as() const {
        if(_interfaces) {
            const Box self = pack();
            sp<U> ptr = _interfaces->as<U>(self);
            if(!ptr) {
                const sp<Duck<U>> duckType = _interfaces->as<Duck<U>>(self);
                if(duckType)
                    duckType->to(ptr);
            }
            return ptr;
        }
        return nullptr;
    }

    bool unique() const {
        return _ptr.unique();
    }

private:
    SharedPtr(T* instance) noexcept
        : _ptr(instance), _interfaces(new Interfaces(Class::getClass<T>())) {
    }
    SharedPtr(T* ptr, std::shared_ptr<Interfaces> interfaces, std::function<void(T*)> deleter) noexcept
        : _ptr(ptr, std::move(deleter)), _interfaces(std::move(interfaces)) {
    }

    static void packedBoxDestructor(void* inst) {
        SharedPtr<T>* b = reinterpret_cast<SharedPtr<T>*>(inst);
        delete b;
    }

    template<typename U> friend class SharedPtr;

    friend class MemoryPool;
    friend class ObjectPool;

private:
    std::shared_ptr<T> _ptr;
    std::shared_ptr<Interfaces> _interfaces;
};


template<> class SharedPtr<void> {
public:
    SharedPtr() {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    SharedPtr(std::nullptr_t) {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    SharedPtr(std::shared_ptr<void>, std::shared_ptr<Interfaces>) {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    Box pack() const {
        return Box();
    }

    operator bool() const {
        return false;
    }

    template<typename U> SharedPtr<U> cast() const {
        return SharedPtr<U>();
    }

    const std::shared_ptr<Interfaces>& interfaces() const {
        return _interfaces;
    }

    template<typename U> sp<U> as() const {
        return nullptr;
    }

    template<typename U> bool is() const {
        return false;
    }

    static SharedPtr<void>& null() {
        static SharedPtr<void> s;
        return s;
    }

private:
    std::shared_ptr<Interfaces> _interfaces;

};

}

#endif
