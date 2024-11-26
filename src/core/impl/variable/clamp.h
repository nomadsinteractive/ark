#pragma once

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class Clamp final : public Variable<T>::ByUpdate, public WithObserver, Implements<Clamp<T>, Variable<T>, WithObserver> {
public:
    Clamp(sp<Variable<T>> delegate, sp<Variable<T>> min, sp<Variable<T>> max, sp<Observer> observer)
        : Variable<T>::ByUpdate(delegate->val()), WithObserver(std::move(observer)), _delegate(std::move(delegate)), _min(std::move(min)), _max(std::move(max)) {
        doClamp(this->_value);
    }

    virtual bool doUpdate(uint64_t timestamp, T& value) override {
        if(!UpdatableUtil::update(timestamp, _delegate, _min, _max))
            return false;

        doClamp(value);
        return true;
    }

private:
    void doClamp(T& value) {
        value = _delegate->val();
        T t;
        if((t = _min->val()) > value) {
            notify();
            value = t;
        } else if((t = _max->val()) < value) {
            notify();
            value = t;
        }
    }

private:
    sp<Variable<T>> _delegate;
    sp<Variable<T>> _min;
    sp<Variable<T>> _max;
};

}
