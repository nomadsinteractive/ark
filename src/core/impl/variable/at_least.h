#pragma once

#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/components/with_callback.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class AtLeast final : public Variable<T>, public Wrapper<Variable<T>>, public WithCallback, Implements<AtLeast<T>, Variable<T>, Wrapper<Variable<T>>, WithCallback> {
public:
    AtLeast(sp<Variable<T>> delegate, sp<Variable<T>> boundary, sp<Runnable> callback)
         : Wrapper<Variable<T>>(std::move(delegate)), WithCallback(std::move(callback)), _boundary(std::move(boundary)) {
        DASSERT(this->_wrapped && _boundary);
    }

    T val() override {
        T value = this->_wrapped->val();
        T boundary = _boundary->val();
        if(value < boundary) {
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
