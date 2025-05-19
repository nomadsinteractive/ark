#pragma once

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
    void addState(sp<State> state) const;

private:
    void doActionActivate(const StateAction& action);
    void doActionDeactivate(const StateAction& action);

    void doActive(sp<State> state);

    struct Stub;

private:
    StateMachine(sp<Stub> stub);

    Vector<sp<State>> _active_states;

    sp<Stub> _stub;

    friend class StateAction;
    friend class State;
};

};
