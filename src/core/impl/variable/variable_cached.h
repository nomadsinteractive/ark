#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class VariableCached final : public Wrapper<Variable<T>>, public Variable<T>, Implements<VariableCached<T>, Wrapper<Variable<T>>> {
public:
    VariableCached(sp<Variable<T>> delegate)
        :  Wrapper<Variable<T>>(std::move(delegate)), _dirty(true) {
    }

    bool update(uint64_t timestamp) override {
        if(this->_wrapped->update(timestamp)) {
            _dirty = true;
            return true;
        }
        return false;
    }

    T val() override {
        if(_dirty) {
            _dirty = false;
            _cached = this->_wrapped->val();
        }
        return _cached;
    }

private:
    T _cached;
    bool _dirty;
};

}
