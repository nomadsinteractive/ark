#ifndef ARK_CORE_TYPES_SAFE_VAR_H_
#define ARK_CORE_TYPES_SAFE_VAR_H_

#include "core/inf/variable.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class SafeVar {
public:
    typedef decltype(std::declval<T>().val()) ValType;
    typedef T _PtrType;

    SafeVar() noexcept
        : _wrapped(nullptr) {
    }
    SafeVar(sp<T> delegate) noexcept
        : _wrapped(std::move(delegate)) {
    }
    SafeVar(sp<T> delegate, const ValType& defaultVal) noexcept
        : _wrapped(std::move(delegate)), _default_val(defaultVal) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafeVar);

    explicit operator bool() const {
        return static_cast<bool>(_wrapped);
    }

    bool operator == (const sp<T>& other) const {
        return _wrapped != other;
    }
    bool operator != (const sp<T>& other) const {
        return _wrapped != other;
    }

    ValType val() const {
        return _wrapped ? _wrapped->val() : _default_val;
    }

    bool update(uint64_t timestamp) const {
        return _wrapped ? _wrapped->update(timestamp) : false;
    }

    operator const sp<T>&() const {
        return ensure();
    }

    const sp<T>& ensure() const {
        if(!_wrapped)
            _wrapped = ensure_sfinae(nullptr);
        return _wrapped;
    }

    const sp<T>& wrapped() const {
        return _wrapped;
    }

    void reset(sp<T> wrapped) {
        _wrapped = std::move(wrapped);
    }

private:
    template<typename U = T> sp<T> ensure_sfinae(std::enable_if_t<std::is_abstract_v<U>, typename U::Const>*) const {
        return sp<typename U::Const>::make(_default_val);
    }

    template<typename U = T> sp<T> ensure_sfinae(std::enable_if_t<!std::is_abstract_v<U>>*) const {
        return sp<U>::make(_default_val);
    }

    template<typename U = T> sp<T> ensure_sfinae(...) const {
        DFATAL("Constructor for default value required");
        return Null::toSafePtr<U>(nullptr);
    }

private:
    mutable sp<T> _wrapped;
    ValType _default_val;
};

}

#endif
