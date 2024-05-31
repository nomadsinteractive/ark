#pragma once

#include "core/forwarding.h"
#include "core/base/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class Clamp : public Variable<T>::ByUpdate {
public:
    Clamp(const sp<Variable<T>>& delegate, const sp<Variable<T>>& min, const sp<Variable<T>>& max, Notifier notifier)
        : Variable<T>::ByUpdate(delegate->val()), _delegate(delegate), _min(min), _max(max), _notifier(std::move(notifier)) {
        doClamp(this->_value);
    }

    virtual bool doUpdate(uint64_t timestamp, T& value) override {
        if(!UpdatableUtil::update(timestamp, _delegate, _min, _max))
            return false;

        doClamp(value);
        return true;
    }

private:
    void doClamp(T& value) {
        value = _delegate->val();
        T t;
        if((t = _min->val()) > value) {
            _notifier.notify();
            value = t;
        } else if((t = _max->val()) < value) {
            _notifier.notify();
            value = t;
        }
    }

private:
    sp<Variable<T>> _delegate;
    sp<Variable<T>> _min;
    sp<Variable<T>> _max;

    Notifier _notifier;
};

}
