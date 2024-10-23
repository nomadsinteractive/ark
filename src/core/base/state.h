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

//  [[script::bindings::auto]]
    sp<StateAction> createAction(const sp<State>& nextState, sp<StateActionGroup> strand = nullptr);

private:
    void doActivate();
    void doDeactivate();

private:
    bool _active;

    sp<Runnable> _on_active;
    sp<Runnable> _on_deactive;

    std::vector<sp<StateAction>> _actions;

    friend class StateMachine;
};

}
