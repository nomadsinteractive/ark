#pragma once

#include <functional>
#include <memory>

#include "core/forwarding.h"
#include "core/types/box.h"
#include "core/types/class.h"
#include "core/types/type.h"

namespace ark {

template<typename T> class SharedPtr {
public:
    constexpr SharedPtr() noexcept = default;
    constexpr SharedPtr(std::nullptr_t) noexcept
        : _ptr(nullptr), _class(nullptr) {
    }
    SharedPtr(std::shared_ptr<T> ptr, const Class* clazz) noexcept
        : _ptr(std::move(ptr)), _class(clazz) {
    }
    template<typename U> SharedPtr(const SharedPtr<U>& ptr) noexcept
        : SharedPtr(std::static_pointer_cast<T>(ptr._ptr), ptr.getClass()) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SharedPtr);

    typedef T _PtrType;
    typedef std::remove_extent_t<T> element_type;

    static SharedPtr<T> adopt(T* instance) {
        return SharedPtr<T>(instance, nullptr);
    }

    template<typename U = T, typename... Args> static SharedPtr<T> make(Args&&... args) {
        return SharedPtr(new U(std::forward<Args>(args)...), std::is_same_v<T, U> ? nullptr : Class::ensureClass<U>());
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

    const Class* getClass() const {
        if(!_class)
            _class = Class::ensureClass<T>();
        return _class;
    }

    template<typename U> SharedPtr<U> cast() const {
        return SharedPtr<U>(std::static_pointer_cast<U>(_ptr), getClass());
    }

    template<typename U> bool isInstance() const {
        return std::is_same_v<T, U> || getClass()->isInstance(Type<U>::id());
    }

    template<typename U> SharedPtr<U> tryCast() const {
        if(_ptr) {
            const Box self(Type<T>::id(), getClass(), this, _ptr.get(), [](const void*) {});
            return self.as<U>();
        }
        return nullptr;
    }

    bool unique() const {
        return _ptr.use_count() == 1;
    }

private:
    SharedPtr(T* instance, const Class* clazz) noexcept
        : _ptr(instance), _class(clazz) {
    }
    SharedPtr(T* ptr, const Class* clazz, std::function<void(T*)> deleter) noexcept
        : _ptr(ptr, std::move(deleter)), _class(clazz) {
    }

    template<typename U> friend class SharedPtr;
    template<typename U> friend class WeakPtr;

private:
    std::shared_ptr<T> _ptr;
    mutable const Class* _class;
};

template<> class SharedPtr<void> {
public:
    SharedPtr() {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    SharedPtr(std::nullptr_t) {
        DFATAL("Illegal SharedPtr<void> class instance creation");
    }

    SharedPtr(std::shared_ptr<void>, const Class*) {
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

    const Class* getClass() const {
        return nullptr;
    }

    template<typename U> sp<U> tryCast() const {
        return nullptr;
    }

    template<typename U> bool is() const {
        return false;
    }
};

}
