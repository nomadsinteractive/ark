#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateMachine {
public:
//  [[script::bindings::auto]]
    StateMachine();

//  [[script::bindings::auto]]
    sp<StateAction> addCommand(sp<Runnable> onActivate = nullptr, sp<Runnable> onDeactivate = nullptr, sp<StateActionGroup> commandGroup = nullptr);

//  [[script::bindings::auto]]
    sp<State> addState(const sp<Runnable>& onActivate = nullptr, const sp<State>& fallback = nullptr);

//  [[script::bindings::auto]]
    void start(State& entry);

//  [[script::bindings::auto]]
    void transit(State& next);

    void activateCommand(StateAction& command);
    void deactivateCommand(StateAction& command);

    const std::vector<sp<State>>& states() const;

private:
    std::vector<sp<State>> _states;
    std::vector<sp<StateAction>> _commands;

    State* _active_state;

    friend class StateAction;
    friend class State;
};

};
