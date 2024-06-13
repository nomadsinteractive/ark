#pragma once

#include <vector>

#include "core/base/wrapper.h"
#include "core/base/observer.h"
#include "core/inf/variable.h"
#include "core/traits/with_observer.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class Expectation final : public Variable<T>, public Wrapper<Variable<T>>, public Implements<Expectation<T>, Variable<T>, Wrapper<Variable<T>>> {
public:
    Expectation(sp<Variable<T>> delegate, sp<WithObserver> withObserver)
        : Wrapper<Variable<T>>(std::move(delegate)), _with_observer(std::move(withObserver)) {
    }

    T val() override {
        return this->_wrapped->val();
    }

    bool update(uint64_t timestamp) override {
        return this->_wrapped->update(timestamp);
    }

    const sp<Observer>& observer() const {
        return _with_observer->observer();
    }

private:
    sp<WithObserver> _with_observer;
};

}
