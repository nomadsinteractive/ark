#pragma once

#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class SafePtr {
public:
    SafePtr() noexcept
        : _inst(Null::toSafePtr<T>(nullptr)), _not_null(false) {
    }
    SafePtr(const sp<T>& inst) noexcept
        : _inst(Null::toSafePtr<T>(inst)), _not_null(static_cast<bool>(inst)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafePtr);

    typedef T _PtrType;

    explicit operator bool() const {
        return _not_null;
    }

    SafePtr& operator =(const sp<T>& other) noexcept {
        _not_null = static_cast<bool>(other);
        _inst = Null::toSafePtr<T>(other);
        return *this;
    }
    SafePtr& operator =(sp<T>&& other) noexcept {
        _not_null = static_cast<bool>(other);
        _inst = _not_null ? std::move(other) : Null::toSafePtr<T>(nullptr);
        return *this;
    }

    T* operator ->() const {
        return _inst.get();
    }

    operator const sp<T>&() const {
        return _inst;
    }

    operator T&() const {
        return *_inst;
    }

private:
    sp<T> _inst;
    bool _not_null;
};

}
