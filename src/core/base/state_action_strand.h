#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state_action.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateActionStrand {
public:
//  [[script::bindings::auto]]
    StateActionStrand(sp<State> start, sp<State> end);

    sp<State> _start;
    sp<State> _end;

private:
    void doActionActive(const StateAction& action);
    const StateAction* doActionDeactive(const StateAction& action);

    std::vector<const StateAction*> _actions;
    std::vector<const StateAction*> _activated_action_stack;

    friend class StateMachine;
    friend class StateAction;
    friend class State;
};

}
