#ifndef ARK_CORE_INF_VARIABLE_H_
#define ARK_CORE_INF_VARIABLE_H_

#include "core/forwarding.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class Variable {
public:
    virtual ~Variable() = default;

    virtual T val() = 0;

    class Const;
    class Impl;
};

template<typename T> class Variable<T>::Impl : public Variable<T>, Implements<typename Variable<T>::Impl, Variable<T>> {
public:
    Impl(const T& value)
        : _value(value) {
    }

    virtual T val() override {
        return _value;
    }

    void set(const T& value) {
        _value = value;
    }

private:
    T _value;
};

template<typename T> class Variable<T>::Const : public Variable<T>, Implements<typename Variable<T>::Const, Variable<T>> {
public:
    Const(const T& value)
        : _value(value) {
    }
    Const(Variable<T>& other)
        : _value(other.val()) {
    }

    virtual T val() override {
        return _value;
    }

private:
    T _value;
};


}

#endif
