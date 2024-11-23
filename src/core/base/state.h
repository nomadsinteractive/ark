#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API State {
public:
//  [[script::bindings::auto]]
    State(sp<Runnable> onActivate = nullptr, sp<Runnable> onDeactivate = nullptr);

//  [[script::bindings::property]]
    bool active() const;

private:
    void doActivate();
    void doDeactivate();

private:
    sp<Runnable> _on_activate;
    sp<Runnable> _on_deactivate;
    bool _active;

    friend class StateMachine;
    friend class StateAction;
};

}
