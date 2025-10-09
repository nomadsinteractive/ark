#pragma once

#include "core/base/timestamp.h"
#include "core/base/wrapper.h"
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
        return static_cast<bool>(this->_stub->wrapped());
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
        return this->_stub.template cast<T>();
    }

    sp<Wrapper<T>> toWrapper() const {
        return this->_stub.template cast<Wrapper<T>>();
    }

    const sp<T>& wrapped() const {
        return this->_stub->wrapped();
    }

    void reset(sp<T> delegate) {
        this->_stub->reset(std::move(delegate));
    }

    void reset(ValType value) {
        this->_stub->reset(std::move(value));
    }

private:
    class Stub final : public Wrapper<T>, public Variable<ValType> {
    public:
        Stub(sp<T> delegate, ValType defaultVal) noexcept
            : Wrapper<T>(std::move(delegate)), _default_val(std::move(defaultVal)) {
        }

        bool update(uint64_t timestamp) override {
            return (this->_wrapped ? this->_wrapped->update(timestamp) : false) || _timestamp.update(timestamp);
        }

        ValType val() override {
            return this->_wrapped ? this->_wrapped->val() : _default_val;
        }

        void reset(sp<T> delegate) {
            this->_wrapped = std::move(delegate);
            _timestamp.markDirty();
        }

        void reset(ValType value) {
            _default_val = std::move(value);
            this->_wrapped = nullptr;
            _timestamp.markDirty();
        }

        ValType _default_val;
        Timestamp _timestamp;
    };

private:
    sp<Stub> _stub;
};

}
