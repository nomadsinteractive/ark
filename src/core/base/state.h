#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state_action.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

namespace ark {

class ARK_API State {
public:
//  [[script::bindings::auto]]
    State(sp<Runnable> onActive = nullptr, sp<Runnable> onDeactivate = nullptr);

//  [[script::bindings::property]]
    bool active() const;

private:
    void doActivate();
    void doDeactivate();

private:
    sp<Runnable> _on_active;
    sp<Runnable> _on_deactive;
    bool _active;

    friend class StateMachine;
    friend class StateAction;
};

}
