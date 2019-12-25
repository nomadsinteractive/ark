#ifndef ARK_CORE_IMPL_VARIABLE_VARIABLE_OBSERVER_H_
#define ARK_CORE_IMPL_VARIABLE_VARIABLE_OBSERVER_H_

#include "core/base/observer.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableObserver : public Variable<T> {
public:
    VariableObserver(const sp<Variable<T>>& delegate, const sp<Observer>& observer)
        : _delegate(delegate), _observer(observer) {
    }

    virtual T val() override {
        return _delegate->val();
    }

    virtual bool update(uint64_t timestamp) override {
        if(_delegate->update(timestamp)) {
            _observer->update();
            return true;
        }
        return false;
    }

private:
    sp<Variable<T>> _delegate;
    sp<Observer> _observer;
};

}

#endif
