#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateAction {
public:
//  [[script::bindings::auto]]
    StateAction(StateMachine& stateMachine, const sp<State>& start, const sp<State>& end, const sp<StateActionStrand>& actionStrand = nullptr);

//  [[script::bindings::property]]
    const sp<State>& start() const;
//  [[script::bindings::property]]
    const sp<State>& end() const;

//  [[script::bindings::auto]]
    void execute() const;
//  [[script::bindings::auto]]
    void activate() const;
//  [[script::bindings::auto]]
    void deactivate() const;

private:
    StateMachine& _state_machine;

    sp<StateActionStrand> _strand;

    friend class StateMachine;
};

}
