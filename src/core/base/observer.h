#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Observer final {
public:
//  [[script::bindings::auto]]
    Observer() = default;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Observer);

//  [[script::bindings::auto]]
    void notify();
//  [[script::bindings::auto]]
    void addCallback(sp<Runnable> callback, bool oneshot = false, uint32_t triggerAfter = 1);
//  [[script::bindings::auto]]
    sp<Boolean> addBooleanSignal(bool value = false, bool oneshot = true);

private:
    struct Callback {
        sp<Runnable> _func;
        bool _oneshot;
        bool _owned;
        uint32_t _trigger_after;
    };

private:
    Vector<Callback> _callbacks;
};

}
