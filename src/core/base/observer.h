#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Observer final {
public:
//  [[script::bindings::auto]]
    Observer(bool oneshot = true);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Observer);

//  [[script::bindings::auto]]
    void notify();

//  [[script::bindings::auto]]
    void addCallback(sp<Runnable> callback, uint32_t triggerAfter = 1);

//  [[script::bindings::auto]]
    sp<Boolean> addBooleanSignal(bool value = false);

private:
    struct Callback {
        sp<Runnable> _func;
        bool _oneshot;
        bool _owned;
        uint32_t _trigger_after;
    };

private:
    std::vector<Callback> _callbacks;
    bool _oneshot;
};

}
