#ifndef ARK_CORE_IMPL_VARIABLE_PERIODIC_H_
#define ARK_CORE_IMPL_VARIABLE_PERIODIC_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/implements.h"
#include "core/util/variable_util.h"

namespace ark {

template<typename T> class Periodic : public Variable<T>::Updatable, public Wrapper<Variable<T>>, public Implements<Periodic<T>, Variable<T>, Wrapper<Variable<T>>> {
public:
    Periodic(sp<Variable<T>> delegate, sp<Numeric> interval, sp<Numeric> duration)
        : Variable<T>::Updatable(delegate->val()), Wrapper<Variable<T>>(std::move(delegate)), _interval(std::move(interval)), _duration(std::move(duration)),
          _next_update_time(_duration->val() + _interval->val()) {
    }

    virtual bool doUpdate(uint64_t timestamp, T& value) override {
        if(!VariableUtil::update(timestamp, this->_wrapped, _interval, _duration))
            return false;

        float d = _duration->val();
        if(d > _next_update_time) {
            _next_update_time = d + _interval->val();
            value = this->_wrapped->val();
        }
        return true;
    }

private:
    sp<Numeric> _interval;
    sp<Numeric> _duration;

    float _next_update_time;
};

}

#endif
