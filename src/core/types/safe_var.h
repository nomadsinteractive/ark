#pragma once

#include "core/base/timestamp.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class SafeVar {
public:
    typedef decltype(std::declval<T>().val()) ValType;
    typedef typename Variable<ValType>::Const WrapperType;
    typedef T _PtrType;

    SafeVar() noexcept
        : _stub(sp<Stub>::make(nullptr, ValType())) {
    }
    SafeVar(nullptr_t) noexcept
        : SafeVar() {
    }
    SafeVar(sp<T> delegate) noexcept
        : SafeVar(std::move(delegate), ValType()) {
    }
    SafeVar(sp<T> delegate, ValType defaultVal) noexcept
        : _stub(sp<Stub>::make(std::move(delegate), std::move(defaultVal))) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafeVar);

    explicit operator bool() const {
        return static_cast<bool>(this->_stub->_delegate);
    }

    ValType val() const {
        return this->_stub->val();
    }

    bool update(uint64_t timestamp) const {
        return this->_stub->update(timestamp);
    }

    explicit operator sp<T>() const {
        return this->_stub;
    }

    sp<T> toVar() const {
        return this->_stub;
    }

    const sp<T>& wrapped() const {
        return this->_stub->_delegate;
    }

    sp<T> ensure() {
        this->_stub->ensure();
        return this->_stub;
    }

    void reset(sp<T> delegate) {
        this->_stub->reset(std::move(delegate));
    }

    void reset(ValType value) {
        this->_stub->reset(std::move(value));
    }

private:
    struct Stub final : Variable<ValType> {
        Stub(sp<T> delegate, ValType defaultVal) noexcept
            : _delegate(std::move(delegate)), _default_val(std::move(defaultVal)) {
        }

        bool update(uint64_t timestamp) override {
            return (_delegate ? _delegate->update(timestamp) : false) || _timestamp.update(timestamp);
        }

        ValType val() override {
            return _delegate ? _delegate->val() : _default_val;
        }

        void reset(sp<T> delegate) {
            _delegate = std::move(delegate);
            _timestamp.markDirty();
        }

        void reset(ValType value) {
            _default_val = std::move(value);
            _delegate = nullptr;
            _timestamp.markDirty();
        }

        void ensure() {
            if(!_delegate) {
                if constexpr(std::is_abstract_v<T>)
                    _delegate = sp<WrapperType>::make(_default_val);
                else
                    _delegate = sp<T>::make(_default_val);
                _timestamp.markDirty();
            }
        }

        sp<T> _delegate;
        ValType _default_val;
        Timestamp _timestamp;
    };

private:
    sp<Stub> _stub;
};

}
