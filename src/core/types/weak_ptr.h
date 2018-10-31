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
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(WeakPtr);

    SharedPtr<T> lock() const {
        std::shared_ptr<T> ptr = _weak_ptr.lock();
        if(!ptr)
            return nullptr;

        std::shared_ptr<Interfaces> interfaces = _weak_interfaces.lock();
        return SharedPtr<T>(std::move(ptr), std::move(interfaces ? interfaces : std::make_shared<Interfaces>(nullptr)));
    }

    SharedPtr<T> ensure() const {
        const SharedPtr<T> ptr = lock();
        DASSERT(ptr);
        return ptr;
    }

    bool expired() const {
        return _weak_ptr.expired();
    }

    long useCount() const {
        return _weak_ptr.use_count();
    }

private:
    std::weak_ptr<T> _weak_ptr;
    std::weak_ptr<Interfaces> _weak_interfaces;
};

}

#endif
