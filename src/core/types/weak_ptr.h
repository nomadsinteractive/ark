#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class WeakPtr {
public:
    constexpr WeakPtr() noexcept = default;
    WeakPtr(const SharedPtr<T>& sharedPtr) noexcept
        : _weak_ptr(sharedPtr._ptr), _weak_interfaces(sharedPtr.ensureInterfaces()) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(WeakPtr);

    void reset(SharedPtr<T> sharedPtr) {
        _weak_interfaces = sharedPtr.ensureInterfaces();
        _weak_ptr = std::move(sharedPtr._ptr);
    }

    SharedPtr<T> lock() const {
        std::shared_ptr<T> ptr = _weak_ptr.lock();
        if(!ptr)
            return nullptr;

        return SharedPtr<T>(std::move(ptr), _weak_interfaces.lock());
    }

    SharedPtr<T> ensure() const {
        SharedPtr<T> ptr = lock();
        DASSERT(ptr);
        return ptr;
    }

    bool expired() const {
        return _weak_ptr.expired();
    }

    bool unique() const {
        return _weak_ptr.use_count() == 1;
    }

    long useCount() const {
        return _weak_ptr.use_count();
    }

private:
    std::weak_ptr<T> _weak_ptr;
    std::weak_ptr<Interfaces> _weak_interfaces;
};

}
