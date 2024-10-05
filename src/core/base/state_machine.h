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
    sp<Command> addCommand(sp<Runnable> onActivate = nullptr, sp<Runnable> onDeactivate = nullptr, sp<CommandGroup> commandGroup = nullptr);

//  [[script::bindings::auto]]
    sp<State> addState(const sp<Runnable>& onActivate = nullptr, const sp<State>& fallback = nullptr);

//  [[script::bindings::auto]]
    void start(State& entry);

//  [[script::bindings::auto]]
    void transit(State& next);

    void activateCommand(Command& command);
    void deactivateCommand(Command& command);

    const std::vector<sp<State>>& states() const;

private:
    std::vector<sp<State>> _states;
    std::vector<sp<Command>> _commands;

    State* _active_state;

    friend class Command;
    friend class State;
};

};
