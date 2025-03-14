#pragma once

#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Future {
public:
//  [[script::bindings::auto]]
    Future(sp<Boolean> canceled = nullptr, sp<Runnable> observer = nullptr);

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
    sp<Runnable> _observer;
    bool _done;
};

}
