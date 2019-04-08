#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OBSERVER_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OBSERVER_H_

#include "core/base/observer.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableObserver : public Variable<T> {
public:
    VariableObserver(const sp<Variable<T>>& delegate, const sp<Observer>& observer)
        : _delegate(delegate), _observer(observer), _value(delegate->val()) {
    }

    virtual T val() override {
        T v = _delegate->val();
        if(v != _value) {
            _value = v;
            _observer->update();
        }
        return v;
    }

private:
    sp<Variable<T>> _delegate;
    sp<Observer> _observer;
    T _value;
};

}

#endif
