#pragma once

#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/state_action.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

namespace ark {

class ARK_API State {
public:
    State(StateMachine& stateMachine, const sp<Runnable>& onActive, State* fallback = nullptr);

//  [[script::bindings::property]]
    bool active() const;

    void activate();
    void deactivate();

    void activate(StateAction& command) const;
    void deactivate(StateAction& command) const;

//  [[script::bindings::auto]]
    void linkCommand(StateAction& command);
//  [[script::bindings::auto]]
    void linkCommandGroup(StateActionGroup& commandGroup);

    int32_t resolveConflicts(const StateAction& command, StateAction::State state, StateAction::State toState) const;

private:
    sp<Runnable> _on_active;
    State* _fallback;
    bool _active;

    std::unordered_map<StateAction*, State*> _linked_commands;

    friend class StateMachine;
};

}
