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

private:
    void doActionActive(const StateAction& action);
    bool doActionDeactive(const StateAction& action);

    sp<State> _start;
    sp<State> _end;

    std::vector<sp<StateAction::Stub>> _activated_actions;

    friend class StateMachine;
    friend class StateAction;
    friend class State;
};

}
