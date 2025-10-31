#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class VariableDirty final : public Wrapper<Variable<T>>, public Boolean, Implements<VariableDirty<T>, Wrapper<Variable<T>>, Boolean> {
public:
    VariableDirty(sp<Variable<T>> delegate)
        :  Wrapper<Variable<T>>(std::move(delegate)), _dirty(false) {
    }

    bool update(uint32_t tick) override {
        if(this->_wrapped->update(tick)) {
            _dirty = true;
            return true;
        }
        return false;
    }

    bool val() override {
        if(_dirty) {
            _dirty = false;
            return true;
        }
        return false;
    }

private:
    bool _dirty;
};

}
