#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"

namespace ark {

template<typename T> class VariableDirtyCallback final : public Wrapper<Variable<T>>, public Variable<T>, Implements<VariableDirtyCallback<T>, Wrapper<Variable<T>>, Variable<T>> {
public:
    VariableDirtyCallback(sp<Variable<T>> delegate, sp<Runnable> callback)
        :  Wrapper<Variable<T>>(std::move(delegate)), _last_value(this->_wrapped->val()), _last_notified(0), _callback(std::move(callback)) {
    }

    bool update(uint32_t tick) override {
        if(this->_wrapped->update(tick) && _last_notified < tick) {
            const T value = this->_wrapped->val();
            if(_last_value != value) {
                _last_value = value;
                _callback->run();
            }
            _last_notified = tick;
            return true;
        }
        return false;
    }

    T val() override {
        return this->_wrapped->val();
    }

private:
    T _last_value;
    uint32_t _last_notified;
    sp<Runnable> _callback;
};

}
