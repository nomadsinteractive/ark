#pragma once

#include "core/inf/variable.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class SafeVar {
public:
    typedef decltype(std::declval<T>().val()) ValType;
    typedef VariableWrapper<ValType> WrapperType;
    typedef T _PtrType;

    SafeVar() noexcept
        : _wrapped(nullptr) {
    }
    SafeVar(sp<T> delegate) noexcept
        : SafeVar(std::move(delegate), ValType()) {
    }
    SafeVar(sp<T> delegate, const ValType& defaultVal) noexcept
        : _default_val(defaultVal), _wrapped(delegate ? ensure_sfinae(std::move(delegate), nullptr) : nullptr) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafeVar);

    explicit operator bool() const {
        return static_cast<bool>(_wrapped);
    }

    ValType val() const {
        return _wrapped ? _wrapped->val() : _default_val;
    }

    bool update(uint64_t timestamp) const {
        return _wrapped ? _wrapped->update(timestamp) : false;
    }

    explicit operator const sp<T>&() const {
        return _wrapped;
    }

    const sp<T>& ensure() const {
        if(!_wrapped)
            _wrapped = ensure_sfinae(nullptr, nullptr);
        return _wrapped;
    }

    const sp<T>& wrapped() const {
        return _wrapped;
    }

    void reset(sp<T> delegate) {
        if(!delegate) {
            _wrapped = nullptr;
            return;
        }

        if(!_wrapped)
            _wrapped = ensure_sfinae(std::move(delegate), nullptr);
        else {
            if constexpr(std::is_abstract_v<T>)
                _wrapped.cast<WrapperType>()->reset(std::move(delegate));
            else
                _wrapped->reset(std::move(delegate));
        }
    }

private:
    template<typename U = T> sp<T> ensure_sfinae(sp<T> delegate, std::enable_if_t<std::is_abstract_v<U>>*) const {
        return delegate ? sp<WrapperType>::make(std::move(delegate)) : sp<WrapperType>::make(_default_val);
    }

    template<typename U = T> sp<T> ensure_sfinae(sp<T> delegate, std::enable_if_t<!std::is_abstract_v<U>>*) const {
        return delegate ? sp<U>::make(*delegate) : sp<U>::make(_default_val);
    }

    template<typename U = T> sp<T> ensure_sfinae(...) const {
        DFATAL("Constructor for default value required");
        return nullptr;
    }

private:
    ValType _default_val;
    mutable sp<T> _wrapped;
};

}
