#pragma once

#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/traits/with_observer.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class VariableExpect final : public Variable<T>, public Wrapper<Variable<T>>, public WithObserver, Implements<VariableExpect<T>, Variable<T>, Wrapper<Variable<T>>, WithObserver> {
public:
    VariableExpect(sp<Variable<T>> delegate, sp<Boolean> expectation, sp<Observer> observer)
         : Wrapper<Variable<T>>(std::move(delegate)), WithObserver(std::move(observer)), _expectation(std::move(expectation)), _value(this->_wrapped->val()) {
    }

    T val() override {
        if(_expectation->val())
            notify();
        return _value;
    }

    bool update(uint64_t timestamp) override {
        bool dirty;
        if(dirty = this->_wrapped->update(timestamp))
            _value = this->_wrapped->val();
        _expectation->update(timestamp);
        return dirty;
    }

private:
    sp<Boolean> _expectation;
    T _value;
};

}
