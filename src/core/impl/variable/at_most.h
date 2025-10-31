#pragma once

#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/components/with_callback.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class AtMost final : public Variable<T>, public Wrapper<Variable<T>>, public WithCallback, Implements<AtMost<T>, Variable<T>, Wrapper<Variable<T>>, WithCallback> {
public:
    AtMost(sp<Variable<T>> delegate, sp<Variable<T>> boundary, sp<Runnable> callback)
         : Wrapper<Variable<T>>(std::move(delegate)), WithCallback(std::move(callback)), _boundary(std::move(boundary)) {
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

    bool update(uint32_t tick) override {
        return UpdatableUtil::update(tick, this->_wrapped, _boundary);
    }

private:
    sp<Variable<T>> _boundary;
};

}
