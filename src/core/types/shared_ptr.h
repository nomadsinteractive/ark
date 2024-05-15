#pragma once

#include <functional>
#include <memory>

#include "core/forwarding.h"
#include "core/inf/duck.h"
#include "core/types/box.h"
#include "core/types/class.h"
#include "core/types/interfaces.h"
#include "core/types/type.h"

namespace ark {

template<typename T> class SharedPtr {
public:
    constexpr SharedPtr() noexcept
        : _ptr(nullptr), _interfaces(nullptr) {
    }
    constexpr SharedPtr(std::nullptr_t null) noexcept
        : _ptr(null), _interfaces(null) {
    }
    SharedPtr(std::shared_ptr<T> ptr, std::shared_ptr<Interfaces> interfaces) noexcept
        : _ptr(std::move(ptr)), _interfaces(std::move(interfaces)) {
    }
    template<typename U> SharedPtr(const SharedPtr<U>& ptr) noexcept
        : SharedPtr(std::static_pointer_cast<T>(ptr._ptr), ptr.ensureInterfaces()) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SharedPtr);

    typedef T _PtrType;
    typedef typename std::remove_extent<T>::type element_type;

    static SharedPtr<T> adopt(T* instance) {
        return SharedPtr<T>(instance);
    }

    template<typename U = T, typename... Args> static SharedPtr<T> make(Args&&... args) {
        return SharedPtr<T>(new U(std::forward<Args>(args)...));
    }

    static const SharedPtr<T>& null() {
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
        DASSERT(_ptr.get());
        return _ptr.get();
    }

    operator T&() const {
        DASSERT(_ptr.get());
        return *_ptr.get();
    }

    T& operator *() const {
        DASSERT(_ptr.get());
        return *_ptr.get();
    }

    T* get() const {
        return _ptr.get();
    }

    const std::shared_ptr<Interfaces>& ensureInterfaces() const {
        if(!_interfaces)
            _interfaces.reset(new Interfaces(Class::getClass<T>()));
        return _interfaces;
    }

    template<typename U> SharedPtr<U> cast() const {
        return SharedPtr<U>(std::static_pointer_cast<U>(_ptr), ensureInterfaces());
    }

//  [[deprecated]]
    template<typename U> const SharedPtr<T>& absorb(const SharedPtr<U>& beingAbsorbed) const {
        ensureInterfaces();
        if(beingAbsorbed._interfaces != _interfaces)
            _interfaces->absorb(beingAbsorbed);
        return *this;
    }

    template<typename U> bool is() const {
        return _interfaces ? _interfaces->isInstance<U>() : std::is_same<T, U>::value;
    }

    template<typename U> SharedPtr<U> as() const {
        if(_ptr) {
            Box self(this, get(), Type<T>::id(), ensureInterfaces(), [](const void*) {});
            return self.as<U>();
        }
        return nullptr;
    }

    bool unique() const {
        return _ptr.use_count() == 1;
    }

private:
    SharedPtr(T* instance) noexcept
        : _ptr(instance) {
    }
    SharedPtr(T* ptr, std::shared_ptr<Interfaces> interfaces, std::function<void(T*)> deleter) noexcept
        : _ptr(ptr, std::move(deleter)), _interfaces(std::move(interfaces)) {
    }

    template<typename U> friend class SharedPtr;
    template<typename U> friend class WeakPtr;

private:
    std::shared_ptr<T> _ptr;
    mutable std::shared_ptr<Interfaces> _interfaces;
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

    void* get() const {
        return nullptr;
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

    const std::shared_ptr<Interfaces>& ensureInterfaces() const {
        return _interfaces;
    }

    template<typename U> sp<U> as() const {
        return nullptr;
    }

    template<typename U> bool is() const {
        return false;
    }

    static const SharedPtr<void>& null() {
        static SharedPtr<void> s;
        return s;
    }

private:
    std::shared_ptr<Interfaces> _interfaces;

};

}
