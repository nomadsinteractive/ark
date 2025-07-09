#pragma once

#include "core/forwarding.h"
#include "core/base/observer.h"
#include "core/inf/variable.h"
#include "core/components/with_callback.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class Fence final : public Variable<T>, public WithCallback, Implements<Fence<T>, Variable<T>, WithCallback> {
public:
    Fence(sp<Variable<T>> delegate, sp<Variable<T>> expectation, sp<Runnable> callback)
        : WithCallback(std::move(callback)), _value(delegate->val()), _delegate(std::move(delegate)), _expectation(std::move(expectation)), _is_greater(this->val() > expectation->val()) {
    }

    T val() override {
        return _value;
    }

    bool update(uint64_t timestamp) override {
        if(!UpdatableUtil::update(timestamp, _delegate, _expectation))
            return false;

        _value = _delegate->val();
        const T boundary = _expectation->val();
        if(const bool isGreater = _value > boundary; isGreater != _is_greater) {
            _is_greater = isGreater;
            notify();
        }
        return true;
    }

private:
    T _value;
    sp<Variable<T>> _delegate;
    sp<Variable<T>> _expectation;
    bool _is_greater;
};

}
