#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API StateAction {
public:
    StateAction(State& start, State& end, sp<StateActionGroup> actionStrand = nullptr);

//  [[script::bindings::auto]]
    void execute();
//  [[script::bindings::auto]]
    void activate();
//  [[script::bindings::auto]]
    void deactivate();

    bool conflicts(const StateAction& other) const;

    const sp<StateActionGroup>& commandGroup() const;

private:
    State& _start;
    State& _end;

    sp<StateActionGroup> _action_strand;
};

}
