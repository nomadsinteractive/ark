#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Runnable)]]
class ARK_API Observer final : public Runnable {
public:
//  [[script::bindings::auto]]
    Observer() = default;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Observer);

    void run() override;

//  [[script::bindings::auto]]
    void notify();
//  [[script::bindings::auto]]
    void addFutureCallback(sp<Future> future);
//  [[script::bindings::auto]]
    sp<Boolean> addBooleanSignal(bool value = false);

private:
    Vector<sp<Future>> _future_callbacks;
};

}
