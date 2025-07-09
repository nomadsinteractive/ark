#pragma once

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class Clamp final : public Variable<T>, public WithCallback, Implements<Clamp<T>, Variable<T>, WithCallback> {
public:
    Clamp(sp<Variable<T>> delegate, sp<Variable<T>> min, sp<Variable<T>> max, sp<Runnable> callback)
        : WithCallback(std::move(callback)), _delegate(std::move(delegate)), _min(std::move(min)), _max(std::move(max)) {
        doClamp();
    }

    T val() override {
        return _value;
    }

    bool update(uint64_t timestamp) override {
        if(!UpdatableUtil::update(timestamp, _delegate, _min, _max))
            return false;

        doClamp();
        return true;
    }

private:
    void doClamp() {
        T t;
        _value = _delegate->val();
        if((t = _min->val()) > _value) {
            notify();
            _value = t;
        } else if((t = _max->val()) < _value) {
            notify();
            _value = t;
        }
    }

private:
    T _value;
    sp<Variable<T>> _delegate;
    sp<Variable<T>> _min;
    sp<Variable<T>> _max;
};

}
