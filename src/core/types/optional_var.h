#pragma once

#include "core/base/timestamp.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class OptionalVar {
public:
    typedef decltype(std::declval<T>().val()) ValType;
    typedef Variable<ValType>::Const WrapperType;
    typedef T _PtrType;

    OptionalVar() noexcept
        : _stub(sp<Stub>::make(nullptr, ValType())) {
    }
    OptionalVar(nullptr_t) noexcept
        : OptionalVar() {
    }
    OptionalVar(sp<T> delegate) noexcept
        : OptionalVar(std::move(delegate), ValType()) {
    }
    OptionalVar(sp<T> delegate, ValType defaultVal) noexcept
        : _stub(sp<Stub>::make(std::move(delegate), std::move(defaultVal))) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(OptionalVar);

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

    void reset(sp<T> delegate) {
        this->_stub->reset(std::move(delegate));
    }

    void reset(ValType value) {
        this->_stub->reset(std::move(value));
    }

private:
    class Stub final : public Variable<ValType> {
    public:
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

        sp<T> _delegate;
        ValType _default_val;
        Timestamp _timestamp;
    };

private:
    sp<Stub> _stub;
};

}
