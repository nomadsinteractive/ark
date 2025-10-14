#pragma once

#include "core/base/api.h"
#include "core/inf/runnable.h"
#include "core/types/optional_var.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Runnable)]]
class ARK_API Future final : public Runnable {
public:
//  [[script::bindings::auto]]
    Future(sp<Runnable> observer = nullptr, sp<Boolean> canceled = nullptr, uint32_t countDown = 1);

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

//  [[script::bindings::property]]
    const sp<Runnable>& observer() const;
//  [[script::bindings::property]]
    void setObserver(sp<Runnable> observer);

    void run() override;

private:
    sp<Runnable> _observer;
    OptionalVar<Boolean> _done;
    OptionalVar<Boolean> _canceled;
    uint32_t _count_down;
};

}
