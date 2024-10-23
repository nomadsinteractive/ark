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
    void addState(sp<State> state);

private:
    State* _active_state;

    std::vector<sp<State>> _states;

    friend class StateAction;
    friend class State;
};

};
