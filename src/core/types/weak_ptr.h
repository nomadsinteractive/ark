#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class WeakPtr {
public:
    constexpr WeakPtr() noexcept = default;
    WeakPtr(const SharedPtr<T>& sharedPtr) noexcept
        : _weak_ptr(sharedPtr._ptr), _weak_class(sharedPtr.getClass()) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(WeakPtr);

    void reset(const SharedPtr<T>& sharedPtr) {
        _weak_class = sharedPtr.getClass();
        _weak_ptr = {sharedPtr._ptr};
    }

    SharedPtr<T> lock() const {
        std::shared_ptr<T> ptr = _weak_ptr.lock();
        if(!ptr)
            return nullptr;

        return SharedPtr<T>(std::move(ptr), _weak_class);
    }

    SharedPtr<T> ensure() const {
        SharedPtr<T> ptr = lock();
        ASSERT(ptr);
        return ptr;
    }

    T& operator *() const {
        const std::shared_ptr<T> ptr = _weak_ptr.lock();
        ASSERT(ptr);
        return *ptr;
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
    const Class* _weak_class;
};

}
