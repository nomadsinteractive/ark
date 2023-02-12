#ifndef ARK_CORE_BASE_FUTURE_H_
#define ARK_CORE_BASE_FUTURE_H_

#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Future {
public:
//  [[script::bindings::auto]]
    Future(sp<Boolean> canceled = nullptr, sp<Observer> observer = nullptr);

//  [[script::bindings::auto]]
    void cancel();
//  [[script::bindings::auto]]
    void done();

//  [[script::bindings::property]]
    bool isCancelled() const;
//  [[script::bindings::property]]
    bool isDone() const;

//  [[script::bindings::property]]
    sp<Boolean> canceled() const;

private:
    sp<VariableWrapper<bool>> _canceled;
    sp<Observer> _observer;
    bool _done;
};

}

#endif
