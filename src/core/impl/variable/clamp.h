#ifndef ARK_CORE_IMPL_VARIABLE_CLAMP_H_
#define ARK_CORE_IMPL_VARIABLE_CLAMP_H_

#include "core/forwarding.h"
#include "core/base/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/variable_util.h"

namespace ark {

template<typename T> class Clamp : public Variable<T>::Updatable {
public:
    Clamp(const sp<Variable<T>>& delegate, const sp<Variable<T>>& min, const sp<Variable<T>>& max, Notifier notifier)
        : Variable<T>::Updatable(delegate->val()), _delegate(delegate), _min(min), _max(max), _notifier(std::move(notifier)) {
    }

    virtual bool doUpdate(uint64_t timestamp, T& value) override {
        if(!VariableUtil::update(timestamp, _delegate, _min, _max))
            return false;

        value = _delegate->val();
        T t;
        if((t = _min->val()) > value) {
            _notifier.notify();
            value = t;
        } else if((t = _max->val()) < value) {
            _notifier.notify();
            value = t;
        }

        return true;
    }

private:
    sp<Variable<T>> _delegate;
    sp<Variable<T>> _min;
    sp<Variable<T>> _max;

    Notifier _notifier;
};

}

#endif
