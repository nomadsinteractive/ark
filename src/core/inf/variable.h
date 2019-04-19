#ifndef ARK_CORE_INF_VARIABLE_H_
#define ARK_CORE_INF_VARIABLE_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Variable {
public:
    virtual ~Variable() = default;

    virtual T val() = 0;

    class Const;
    class Get;
    class Impl;
    class Synchronized;
};

template<typename T> class Variable<T>::Impl : public Variable<T> {
public:
    Impl(const T& value)
        : _value(value) {
    }
    Impl(Variable<T>& other)
        : _value(other.val()) {
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

template<typename T> class Variable<T>::Const : public Variable<T> {
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


template<typename T> class Variable<T>::Get : public Variable<T> {
public:
    Get(const sp<Dictionary<T>>& dictionary, const String& name)
        : _dictionary(dictionary), _name(name) {
    }

    virtual T val() override {
        return _dictionary->get(_name);
    }

private:
    sp<Dictionary<T>> _dictionary;
    String _name;
};

template<typename T> class Variable<T>::Synchronized : public Variable<T> {
public:
    Synchronized(const sp<Variable<T>>& delegate, const sp<Boolean>& flag)
        : _delegate(delegate), _flag(flag), _cached(_delegate->val()) {
    }

    virtual T val() override {
        if(_flag->val())
            _cached = _delegate->val();
        return _cached;
    }

private:
    sp<Variable<T>> _delegate;
    sp<Boolean> _flag;

    T _cached;
};


}

#endif
