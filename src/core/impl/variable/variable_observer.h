#pragma once

#include "core/base/observer.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableObserver : public Variable<T> {
public:
    VariableObserver(sp<Variable<T>> delegate, sp<Observer> observer)
        : _delegate(std::move(delegate)), _observer(std::move(observer)), _val(_delegate->val()) {
    }

    virtual T val() override {
        return _delegate->val();
    }

    virtual bool update(uint64_t timestamp) override {
        if(_delegate->update(timestamp)) {
            T val = _delegate->val();
            if(val != _val) {
                _val = val;
                _observer->notify();
            }
            return true;
        }
        return false;
    }

private:
    sp<Variable<T>> _delegate;
    sp<Observer> _observer;
    T _val;
};

}
