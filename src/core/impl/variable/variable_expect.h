#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/base/future.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class VariableExpect final : public Wrapper<Variable<T>>, public Variable<T>, Implements<VariableExpect<T>, Wrapper<Variable<T>>, Variable<T>> {
public:
    VariableExpect(sp<Variable<T>> delegate, sp<Variable<T>> expectation, sp<Future> future)
        :  Wrapper<Variable<T>>(std::move(delegate)), _expectation(std::move(expectation)), _future(std::move(future)) {
    }

    bool update(uint32_t tick) override {
        _expectation->update(tick);
        return this->_wrapped->update(tick);
    }

    bool val() override {
        const T val = this->_wrapped->val();
        if(!_future->isDoneOrCanceled()->val() && val == _expectation->val())
            _future->notify();
        return val;
    }

private:
    sp<Variable<T>> _expectation;
    sp<Future> _future;
};

}
