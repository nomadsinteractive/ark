#pragma once

#include "core/base/api.h"
#include "core/inf/runnable.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Runnable)]]
class ARK_API Future : public Runnable {
public:
//  [[script::bindings::auto]]
    Future(sp<Boolean> canceled = nullptr, sp<Runnable> observer = nullptr, uint32_t countDown = 1);

//  [[script::bindings::auto]]
    void cancel();
//  [[script::bindings::auto]]
    void notify();

//  [[script::bindings::property]]
    sp<Boolean> isCanceled() const;
//  [[script::bindings::property]]
    sp<Boolean> isDone() const;
//  [[script::bindings::property]]
    sp<Boolean> isDoneOrCanceled() const;

    void run() override;

private:
    SafeVar<Boolean> _done;
    SafeVar<Boolean> _canceled;
    sp<Runnable> _observer;
    uint32_t _count_down;
};

}
