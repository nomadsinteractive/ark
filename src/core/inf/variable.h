#pragma once

#include "core/forwarding.h"
#include "core/base/timestamp.h"
#include "core/inf/updatable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class Variable : public Updatable {
public:
    ~Variable() override = default;

    virtual T val() = 0;

    class Const;
    class Impl;

    class ByUpdate;
};

template<typename T> class Variable<T>::Impl final : public Variable<T>, Implements<typename Variable<T>::Impl, Variable<T>> {
public:
    Impl(T value)
        : _value(std::move(value)) {
    }

    T val() override {
        return _value;
    }

    bool update(uint32_t tick) override {
        return _timestamp.update(tick);
    }

    void set(T value) {
        _value = std::move(value);
        _timestamp.markDirty();
    }

private:
    T _value;
    Timestamp _timestamp;
};

template<typename T> class Variable<T>::Const final : public Variable<T>, Implements<typename Variable<T>::Const, Variable<T>> {
public:
    Const(T value)
        : _value(std::move(value)) {
    }
    Const(Variable<T>& other)
        : _value(other.val()) {
    }

    T val() override {
        return _value;
    }

    bool update(uint32_t /*tick*/) override {
        return false;
    }

private:
    T _value;
};

}
