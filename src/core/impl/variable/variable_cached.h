#pragma once

#include "core/forwarding.h"
#include "core/base/timestamp.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class VariableCached final : public Wrapper<Variable<T>>, public Variable<T>, Implements<VariableCached<T>, Wrapper<Variable<T>>> {
public:
    VariableCached(sp<Variable<T>> delegate)
        :  Wrapper<Variable<T>>(std::move(delegate)), _last_updated(Timestamp::now()), _value(this->_wrapped->update(_last_updated) ? this->_wrapped->val() : this->_wrapped->val()) {
    }

    bool update(uint32_t tick) override {
        if(this->_wrapped->update(tick)) {
            if(_last_updated != tick) {
                _last_updated = tick;
                _value = this->_wrapped->val();
            }
            return true;
        }
        return false;
    }

    T val() override {
        return _value;
    }

    template<typename U, typename... Args> static sp<Variable<T>> create(Args&&... args) {
        return sp<Variable<T>>::template make<VariableCached>(sp<Variable<T>>::template make<U>(std::forward<Args>(args)...));
    }

private:
    uint32_t _last_updated;
    T _value;
};

}
