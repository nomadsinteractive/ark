#pragma once

#include "core/forwarding.h"
#include "core/base/observer.h"
#include "core/inf/variable.h"
#include "core/components/with_observer.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class Fence final : public Variable<T>::ByUpdate, public WithObserver, Implements<Fence<T>, Variable<T>, WithObserver> {
public:
    Fence(sp<Variable<T>> delegate, sp<Variable<T>> expectation, sp<Observer> observer)
        : Variable<T>::ByUpdate(delegate->val()), WithObserver(std::move(observer)), _delegate(std::move(delegate)), _expectation(std::move(expectation)), _is_greater(this->val() > expectation->val()) {
    }

    bool doUpdate(uint64_t timestamp, T& value) override {
        if(!UpdatableUtil::update(timestamp, _delegate, _expectation))
            return false;

        value = _delegate->val();
        const T boundary = _expectation->val();
        if(const bool isGreater = value > boundary; isGreater != _is_greater) {
            _is_greater = isGreater;
            notify();
        }
        return true;
    }

private:
    sp<Variable<T>> _delegate;
    sp<Variable<T>> _expectation;

    bool _is_greater;
};

}
