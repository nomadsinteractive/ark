#pragma once

#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/traits/with_observer.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class AtMost final : public Variable<T>, public Wrapper<Variable<T>>, public WithObserver, Implements<AtMost<T>, Variable<T>, Wrapper<Variable<T>>, WithObserver> {
public:
    AtMost(sp<Variable<T>> delegate, sp<Variable<T>> boundary, sp<Observer> observer)
         : Wrapper<Variable<T>>(std::move(delegate)), WithObserver(std::move(observer)), _boundary(std::move(boundary)) {
    }

    T val() override {
        T value = this->_wrapped->val();
        T boundary = _boundary->val();
        if(value > boundary) {
            notify();
            return boundary;
        }
        return value;
    }

    bool update(uint64_t timestamp) override {
        return UpdatableUtil::update(timestamp, this->_wrapped, _boundary);
    }

private:
    sp<Variable<T>> _boundary;
};

}
