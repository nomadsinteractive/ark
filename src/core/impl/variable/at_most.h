#ifndef ARK_CORE_IMPL_VARIABLE_AT_MOST_H_
#define ARK_CORE_IMPL_VARIABLE_AT_MOST_H_

#include "core/base/delegate.h"
#include "core/base/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/variable_util.h"

namespace ark {

template<typename T> class AtMost : public Variable<T>, public Delegate<Variable<T>>, Implements<AtMost<T>, Variable<T>, Delegate<Variable<T>>> {
public:
    AtMost(const sp<Variable<T>>& delegate, const sp<Variable<T>>& boundary, Notifier notifier)
         : Delegate<Variable<T>>(delegate), _boundary(boundary), _notifer(std::move(notifier)) {
    }

    virtual T val() override {
        T value = _delegate->val();
        T boundary = _boundary->val();
        if(value > boundary) {
            _notifer.notify();
            return boundary;
        }
        return value;
    }

    virtual bool update(uint64_t timestamp) override {
        return VariableUtil::update(timestamp, _delegate, _boundary);
    }

private:
    sp<Variable<T>> _boundary;

    Notifier _notifer;
};

}

#endif
