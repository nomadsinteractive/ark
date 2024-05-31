#pragma once

#include "core/inf/variable.h"
#include "core/base/wrapper.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class SafeVar : public Wrapper<T> {
public:
    typedef decltype(std::declval<T>().val()) ValType;
    typedef typename Variable<ValType>::Const WrapperType;
    typedef T _PtrType;

    SafeVar() noexcept
        : Wrapper<T>(nullptr), _default_val() {
    }
    SafeVar(nullptr_t) noexcept
        : Wrapper<T>(nullptr), _default_val() {
    }
    SafeVar(sp<T> delegate) noexcept
        : SafeVar(std::move(delegate), ValType()) {
    }
    SafeVar(sp<T> delegate, const ValType& defaultVal) noexcept
        : Wrapper<T>(std::move(delegate)), _default_val(defaultVal) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafeVar);

    explicit operator bool() const {
        return static_cast<bool>(this->_wrapped);
    }

    ValType val() const {
        return this->_wrapped ? this->_wrapped->val() : _default_val;
    }

    bool update(uint64_t timestamp) const {
        return this->_wrapped ? this->_wrapped->update(timestamp) : false;
    }

    explicit operator const sp<T>&() const {
        return this->_wrapped;
    }

    const sp<T>& ensure() {
        if(!this->_wrapped)
            this->_wrapped = ensure_sfinae(nullptr);
        return this->_wrapped;
    }

    void reset(sp<T> delegate) {
        Wrapper<T>::reset(std::move(delegate));
    }

    void reset(ValType value) {
        _default_val = std::move(value);
        this->_wrapped = nullptr;
    }

private:
    sp<T> ensure_sfinae(void*) const {
        if constexpr(std::is_abstract_v<T>)
            return sp<WrapperType>::make(_default_val);
        else
            return sp<T>::make(_default_val);
    }

private:
    ValType _default_val;
};

}
