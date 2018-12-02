#ifndef ARK_CORE_TYPES_SAFE_PTR_H_
#define ARK_CORE_TYPES_SAFE_PTR_H_

#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class SafePtr {
public:
    SafePtr() noexcept
        : _inst(Null::toSafe<T>(nullptr)), _allocated(false) {
    }
    SafePtr(const sp<T>& inst) noexcept
        : _inst(Null::toSafe<T>(inst)), _allocated(static_cast<bool>(inst)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafePtr);

    typedef T _PtrType;

    explicit operator bool() const {
        return _allocated;
    }

    SafePtr& operator =(const sp<T>& other) noexcept {
        _allocated = static_cast<bool>(other);
        _inst = Null::toSafe<T>(other);
        return *this;
    }
    SafePtr& operator =(sp<T>&& other) noexcept {
        _allocated = static_cast<bool>(other);
        _inst = _allocated ? std::move(other) : Null::toSafe<T>(nullptr);
        return *this;
    }

    bool operator != (const sp<T>& other) const {
        return _inst != other._inst;
    }

    T* operator ->() const {
        return _inst.get();
    }

    operator const sp<T>&() const {
        return _inst;
    }

    const sp<T>& ensure() const {
        _allocated = true;
        return _inst;
    }

private:
    sp<T> _inst;
    mutable bool _allocated;
};

}

#endif
