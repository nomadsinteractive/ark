#ifndef ARK_CORE_IMPL_VARIABLE_FENCE_H_
#define ARK_CORE_IMPL_VARIABLE_FENCE_H_

#include "core/forwarding.h"
#include "core/base/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

namespace ark {

template<typename T> class Fence : public Variable<T>::ByUpdate {
public:
    Fence(const sp<Variable<T>>& delegate, const sp<Variable<T>>& expectation, Notifier notifier)
        : Variable<T>::ByUpdate(delegate->val()), _delegate(delegate), _expectation(expectation), _notifer(std::move(notifier)), _is_greater(this->val() > expectation->val()) {
    }

    virtual bool doUpdate(uint64_t timestamp, T& value) override {
        if(!UpdatableUtil::update(timestamp, _delegate, _expectation))
            return false;

        value = _delegate->val();
        T boundary = _expectation->val();
        bool isGreater = value > boundary;
        if(isGreater != _is_greater)
        {
            _is_greater = isGreater;
            _notifer.notify();
        }
        return true;
    }

private:
    sp<Variable<T>> _delegate;
    sp<Variable<T>> _expectation;

    Notifier _notifer;

    bool _is_greater;

};

}

#endif
