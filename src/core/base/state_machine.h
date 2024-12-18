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
    StateMachine(sp<State> entry);

//  [[script::bindings::auto]]
    void reset(sp<State> state);

private:
    void doActionActivate(const StateAction& action);
    void doActionDeactivate(const StateAction& action);

    void doActive(sp<State> state);

private:
    std::vector<sp<State>> _active_states;

    friend class StateAction;
    friend class State;
};

};
