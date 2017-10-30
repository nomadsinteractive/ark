#ifndef ARK_CORE_TYPES_WEAK_PTR_H_
#define ARK_CORE_TYPES_WEAK_PTR_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class WeakPtr {
public:
    _CONSTEXPR WeakPtr() noexcept {
    }
    WeakPtr(const SharedPtr<T>& sharedPtr) noexcept
        : _weak_ptr(sharedPtr.ptr()), _weak_interfaces(sharedPtr.interfaces()) {
    }
    WeakPtr(const WeakPtr<T>& other) noexcept
        : _weak_ptr(other._weak_ptr), _weak_interfaces(other._weak_interfaces) {
    }
    WeakPtr(WeakPtr<T>&& other) noexcept
        : _weak_ptr(std::move(other._weak_ptr)), _weak_interfaces(std::move(other._weak_interfaces)) {
    }

    const WeakPtr<T>& operator =(const WeakPtr<T>& other) {
        _weak_ptr = other._weak_ptr;
        _weak_interfaces = other._weak_interfaces;
        return *this;
    }

    const WeakPtr<T>& operator =(WeakPtr<T>&& other) {
        _weak_ptr = std::move(other._weak_ptr);
        _weak_interfaces = std::move(other._weak_interfaces);
        return *this;
    }

    SharedPtr<T> lock() const {
        const std::shared_ptr<T> ptr = _weak_ptr.lock();
        const std::shared_ptr<Interfaces> interfaces = _weak_interfaces.lock();
        return ptr ? SharedPtr<T>(ptr, interfaces ? interfaces : std::make_shared<Interfaces>()) : nullptr;
    }

    SharedPtr<T> ensure() const {
        const SharedPtr<T> ptr = lock();
        NOT_NULL(ptr);
        return ptr;
    }

private:
    std::weak_ptr<T> _weak_ptr;
    std::weak_ptr<Interfaces> _weak_interfaces;
};

}

#endif
