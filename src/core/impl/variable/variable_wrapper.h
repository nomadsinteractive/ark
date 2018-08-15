#ifndef ARK_CORE_BASE_VARIABLE_WRAPPER_H_
#define ARK_CORE_BASE_VARIABLE_WRAPPER_H_

#include "core/forwarding.h"
#include "core/ark.h"
#include "core/inf/variable.h"

#include "app/base/application_context.h"

namespace ark {

template<typename T> class VariableWrapper final : public Variable<T> {
public:
    VariableWrapper(const sp<Variable<T>>& delegate)
        : _variable_impl(nullptr), _delegate(Null::toSafe(delegate)) {
    }
    VariableWrapper(T value)
        : _variable_impl(new typename Variable<T>::Impl(value)), _delegate(sp<Variable<T>>::adopt(_variable_impl)) {
    }
    VariableWrapper(const VariableWrapper& other) = default;
    VariableWrapper(VariableWrapper&& other) = default;

    virtual T val() override {
        return _delegate->val();
    }

    const sp<Variable<T>>& delegate() const {
        return _delegate;
    }

    void set(T value) {
        if(_variable_impl)
            _variable_impl->set(value);
        else
        {
            deferedUnref();
            _variable_impl = new typename Variable<T>::Impl(value);
            _delegate = sp<Variable<T>>::adopt(_variable_impl);
        }
    }

    void set(const sp<Variable<T>>& delegate) {
        deferedUnref();
        DCHECK(delegate.get() != this, "Recursive delegate being set");
        _delegate = Null::toSafe(delegate);
    }

    T fix() {
        T val = _delegate->val();
        set(val);
        return val;
    }

private:
    void deferedUnref() {
        _variable_impl = nullptr;
        if(_delegate)
            Ark::instance().applicationContext()->deferUnref(std::move(_delegate));
    }

private:
    typename Variable<T>::Impl* _variable_impl;
    sp<Variable<T>> _delegate;
};

}

#endif
