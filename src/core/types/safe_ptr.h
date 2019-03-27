#ifndef ARK_CORE_TYPES_SAFE_PTR_H_
#define ARK_CORE_TYPES_SAFE_PTR_H_

#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class SafePtr {
public:
    SafePtr() noexcept
        : _inst(Null::toSafe<T>(nullptr)), _not_null(false) {
    }
    SafePtr(const sp<T>& inst) noexcept
        : _inst(Null::toSafe<T>(inst)), _not_null(static_cast<bool>(inst)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafePtr);

    typedef T _PtrType;

    explicit operator bool() const {
        return _not_null;
    }

    SafePtr& operator =(const sp<T>& other) noexcept {
        _not_null = static_cast<bool>(other);
        _inst = Null::toSafe<T>(other);
        return *this;
    }
    SafePtr& operator =(sp<T>&& other) noexcept {
        _not_null = static_cast<bool>(other);
        _inst = _not_null ? std::move(other) : Null::toSafe<T>(nullptr);
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

    operator T&() const {
        return *_inst;
    }

    T* get() const {
        return _inst.get();
    }

private:
    sp<T> _inst;
    bool _not_null;
};

}

#endif
