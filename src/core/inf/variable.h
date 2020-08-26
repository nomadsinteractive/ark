#ifndef ARK_CORE_INF_VARIABLE_H_
#define ARK_CORE_INF_VARIABLE_H_

#include "core/forwarding.h"
#include "core/base/timestamp.h"
#include "core/inf/updatable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class Variable : public Updatable {
public:
    virtual ~Variable() = default;

    virtual T val() = 0;

    class Const;
    class Impl;

    class Updatable;
};

template<typename T> class Variable<T>::Impl : public Variable<T>, Implements<typename Variable<T>::Impl, Variable<T>> {
public:
    Impl(T value)
        : _value(std::move(value)) {
    }

    virtual T val() override {
        return _value;
    }

    virtual bool update(uint64_t timestamp) override {
        return _timestamp.update(timestamp);
    }

    void set(const T& value) {
        _value = value;
        _timestamp.setDirty();
    }

private:
    T _value;
    Timestamp _timestamp;
};

template<typename T> class Variable<T>::Const : public Variable<T>, Implements<typename Variable<T>::Const, Variable<T>> {
public:
    Const(T value)
        : _value(std::move(value)) {
    }
    Const(Variable<T>& other)
        : _value(other.val()) {
    }

    virtual T val() override {
        return _value;
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

private:
    T _value;
};

template<typename T> class Variable<T>::Updatable : public Variable<T> {
public:
    Updatable(T value)
        : _value(std::move(value)) {
    }

    virtual T val() override {
        return _value;
    }

    virtual bool update(uint64_t timestamp) override {
        return doUpdate(timestamp, _value);
    }

protected:
    virtual bool doUpdate(uint64_t /*timestamp*/, T& value) = 0;

private:
    T _value;
};


}

#endif
