#pragma once

#include "core/base/wrapper.h"
#include "core/inf/builder.h"

namespace ark {

template<typename T> class SafeBuilder final : public Wrapper<Builder<T>>, public Builder<T> {
public:
    SafeBuilder() = default;
    SafeBuilder(sp<Builder<T>> delegate)
        : Wrapper<Builder<T>>(std::move(delegate)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafeBuilder);

    explicit operator bool() const {
        return static_cast<bool>(this->_wrapped);
    }

    sp<T> build(const Scope& args) override {
        return this->_wrapped ? this->_wrapped->build(args) : nullptr;
    }

    sp<T> build(const Scope& args) const {
        return this->_wrapped ? this->_wrapped->build(args) : nullptr;
    }
};

template<typename T> class SafeIBuilder final : public Wrapper<IBuilder<T>>, public IBuilder<T> {
public:
    SafeIBuilder() = default;
    SafeIBuilder(sp<IBuilder<T>> delegate, T defaultValue = {})
        : Wrapper<IBuilder<T>>(std::move(delegate)), _default_value(std::move(defaultValue)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(SafeIBuilder);

    explicit operator bool() const {
        return static_cast<bool>(this->_wrapped);
    }

    T build(const Scope& args) override {
        return this->_wrapped ? this->_wrapped->build(args) : _default_value;
    }

    T build(const Scope& args) const {
        return this->_wrapped ? this->_wrapped->build(args) : _default_value;
    }

private:
    T _default_value;
};

}
